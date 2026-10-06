#include "pch.h"
#include "splitscreen.h"

namespace t4
{
namespace sp
{
namespace
{
ClientViewParams stockViewParams[8];
const ClientViewParams horizontalViewParams[2] = {{0.0f, 0.0f, 1.0f, 0.5f}, {0.0f, 0.5f, 1.0f, 0.5f}};

Detour SetViewDetour;
Detour UpdateViewDetour;
Detour DrawViewDetour;
const dvar_s *horizontalDvar = nullptr;
bool horizontalLayout = false;

bool HorizontalEnabled()
{
    return horizontalDvar && horizontalDvar->current.enabled;
}

void RemapView(ClientViewParams &view, const ClientViewParams &from, const ClientViewParams &to)
{
    // T4 interpolates each rectangle between its split-screen layout and (0, 0, 1, 1).
    // Height identifies that interpolation even when the horizontal layout has x == 0.
    float fullscreenFraction = (view.height - from.height) / (1.0f - from.height);
    if (fullscreenFraction < 0.0f)
        fullscreenFraction = 0.0f;
    else if (fullscreenFraction > 1.0f)
        fullscreenFraction = 1.0f;
    view.x = to.x * (1.0f - fullscreenFraction);
    view.y = to.y * (1.0f - fullscreenFraction);
    view.width = to.width + (1.0f - to.width) * fullscreenFraction;
    view.height = to.height + (1.0f - to.height) * fullscreenFraction;
}

void ApplyLayout(bool horizontal)
{
    for (int displayMode = 0; displayMode < 2; ++displayMode)
    {
        for (int client = 0; client < 2; ++client)
        {
            const int index = displayMode * 4 + 2 + client;
            const auto &from = horizontalLayout ? horizontalViewParams[client] : stockViewParams[index];
            const auto &to = horizontal ? horizontalViewParams[client] : stockViewParams[index];
            RemapView(startViewParams[index], from, to);
            RemapView(targetViewParams[index], from, to);
            RemapView(currentViewParams[index], from, to);
            defaultViewParams[index] = to;
        }
    }
    horizontalLayout = horizontal;
}

bool UpdateLayout()
{
    const bool horizontal = HorizontalEnabled();
    if (horizontal == horizontalLayout)
        return false;

    ApplyLayout(horizontal);
    DbgPrint("[codxe][T4 SP][SplitScreen] cg_splitScreenHorizontal=%d\n", horizontal);
    return true;
}

unsigned int CG_SetView_Hook(int localClientNum, int activeClientIndex, int activeClientCount)
{
    const bool changed = UpdateLayout();
    if (changed && activeClientCount == 2 && localClientNum >= 0 && localClientNum < 2)
    {
        // Refresh the other player's viewport and HUD placement when toggled during setup.
        SetViewDetour.GetOriginal<CG_SetView_t>()(localClientNum ^ 1, activeClientIndex ^ 1, activeClientCount);
    }
    return SetViewDetour.GetOriginal<CG_SetView_t>()(localClientNum, activeClientIndex, activeClientCount);
}

unsigned int CG_UpdateView_Hook(int time)
{
    const bool changed = UpdateLayout();
    const auto result = UpdateViewDetour.GetOriginal<CG_UpdateView_t>()(time);
    if (changed && CL_LocalClientActiveCount() == 2)
    {
        // The stock updater only calls CG_SetView while an animation changes the rectangle.
        // Recalculate both clients even when they are already at their split-screen endpoints.
        for (int client = 0; client < 2; ++client)
            SetViewDetour.GetOriginal<CG_SetView_t>()(client, client, 2);
    }
    return result;
}

void *RB_DrawView_Hook(uint32_t view)
{
    auto &splitScreenOverlay = (*backEndData)->splitScreenOverlay;
    const auto overlay = splitScreenOverlay;
    // 1 draws splitscreen_sidebars(_wide); 2 draws the fixed 4:3 centre bar.
    // Both masks describe the stock layout and would cover the new viewports.
    if (HorizontalEnabled() && (overlay == 1 || overlay == 2))
        splitScreenOverlay = 0;
    const auto result = DrawViewDetour.GetOriginal<RB_DrawView_t>()(view);
    splitScreenOverlay = overlay;
    return result;
}
} // namespace

void SplitScreen::OnDvarInit()
{
    horizontalDvar = Dvar_RegisterBool("cg_splitScreenHorizontal", false, DVAR_FLAG_NONE,
                                       "Use full-width top and bottom views for two players");
}

SplitScreen::SplitScreen()
{
    std::memcpy(stockViewParams, defaultViewParams, sizeof(stockViewParams));

    SetViewDetour = Detour(CG_SetView, CG_SetView_Hook);
    SetViewDetour.Install();

    UpdateViewDetour = Detour(CG_UpdateView, CG_UpdateView_Hook);
    UpdateViewDetour.Install();

    DrawViewDetour = Detour(RB_DrawView, RB_DrawView_Hook);
    DrawViewDetour.Install();
}

SplitScreen::~SplitScreen()
{
    DrawViewDetour.Remove();
    UpdateViewDetour.Remove();
    SetViewDetour.Remove();
    if (horizontalLayout)
        ApplyLayout(false);
}
} // namespace sp
} // namespace t4
