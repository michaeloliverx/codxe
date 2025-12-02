#include "pch.h"
#include "cg.h"

namespace iw4
{
namespace mp
{

void DrawBranding(int localClientNum)
{
    auto const placement = ScrPlace_GetUnsafeFullPlacement();

    const char *text = branding::GetBrandingString(branding::GAME_IW4, branding::MODE_MP);

    const float color_white_rgba[4] = {1.0f, 1.0f, 1.0f, 0.5f}; // RGBA white color

    const float x = -100.0f;
    const float y = 10.0f;

    UI_DrawText(placement, text, 64, sharedUiInfo->assets.consoleFont, x, y, 0, 0, 0.2, color_white_rgba, 0);
}

Detour UI_DrawBuildNumber_Detour;

void UI_DrawBuildNumber_Hook(int localClientNum)
{
    UI_DrawBuildNumber_Detour.GetOriginal<decltype(UI_DrawBuildNumber)>()(localClientNum);
    DrawBranding(localClientNum);
}

cg::cg()
{
    UI_DrawBuildNumber_Detour = Detour(UI_DrawBuildNumber, UI_DrawBuildNumber_Hook);
    UI_DrawBuildNumber_Detour.Install();
}

cg::~cg()
{
    UI_DrawBuildNumber_Detour.Remove();
}
} // namespace mp
} // namespace iw4
