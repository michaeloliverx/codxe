#include "pch.h"
#include "branding.h"

namespace iw4
{
namespace mp
{
namespace
{
Detour UI_DrawBuildNumber_Detour;
} // namespace

void Branding::DrawBranding(int localClientNum)
{
    const ScreenPlacement *placement = ScrPlace_GetActivePlacement(localClientNum);
    const char *text = branding::GetBrandingString();
    const float color_white_rgba[4] = {1.0f, 1.0f, 1.0f, 0.5f};

    const float x = -100.0f;
    const float y = 10.0f;

    UI_DrawText(placement, text, 64, sharedUiInfo->assets.consoleFont, x, y, 0, 0, 0.2, color_white_rgba, 0);
}

void Branding::UI_DrawBuildNumber_Hook(int localClientNum)
{
    UI_DrawBuildNumber_Detour.GetOriginal<decltype(UI_DrawBuildNumber)>()(localClientNum);
    DrawBranding(localClientNum);
}

Branding::Branding()
{
    UI_DrawBuildNumber_Detour = Detour(UI_DrawBuildNumber, Branding::UI_DrawBuildNumber_Hook);
    UI_DrawBuildNumber_Detour.Install();
}

Branding::~Branding()
{
    UI_DrawBuildNumber_Detour.Remove();
}
} // namespace mp
} // namespace iw4
