#include "cg.h"
#include "common.h"

namespace iw4
{
namespace sp
{

void DrawBranding(int localClientNum)
{
    auto const placement = ScrPlace_GetUnsafeFullPlacement();

    const char *text = branding::GetBrandingString(branding::GAME_IW4, branding::MODE_SP);

    const float color_white_rgba[4] = {1.0f, 1.0f, 1.0f, 0.5f}; // RGBA white color

    const float x = -100.0f;
    const float y = 10.0f;

    UI_DrawText(placement, text, 64, sharedUiInfo->assets.consoleFont, x, y, 0, 0, 0.2, color_white_rgba, 0);
}

Detour UI_RefreshViewport_Detour;

void UI_RefreshViewport_Hook(int localClientNum)
{
    UI_RefreshViewport_Detour.GetOriginal<decltype(UI_RefreshViewport)>()(localClientNum);
    DrawBranding(localClientNum);
}

cg::cg()
{
    UI_RefreshViewport_Detour = Detour(UI_RefreshViewport, UI_RefreshViewport_Hook);
    UI_RefreshViewport_Detour.Install();
}

cg::~cg()
{
    UI_RefreshViewport_Detour.Remove();
}
} // namespace sp
} // namespace iw4
