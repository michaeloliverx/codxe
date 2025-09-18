#include "branding.h"

void DrawBranding(int localClientNum)
{
    const iw5::mp::ScreenPlacement *placement =
        iw5::mp::ScrPlace_GetActivePlacement((iw5::mp::LocalClientNum_t)localClientNum);
    const char *text = branding::GetBrandingString(branding::GAME_IW5, branding::MODE_MP);
    const float color_white_rgba[4] = {1.0f, 1.0f, 1.0f, 0.5f};

    const float x = -105.0f;
    const float y = 10.0f;

    iw5::mp::UI_DrawText(placement, text, 64, iw5::mp::sharedUiInfo->assets.consoleFont, x, y, 0, 0, 0.2,
                         color_white_rgba, 0);
}

Detour UI_DrawBuildNumber_Detour;

void UI_DrawBuildNumber_Hook(iw5::mp::LocalClientNum_t localClientNum)
{
    UI_DrawBuildNumber_Detour.GetOriginal<iw5::mp::UI_DrawBuildNumber_t>()(localClientNum);
    DrawBranding(localClientNum);
}

Branding::Branding()
{
    UI_DrawBuildNumber_Detour = Detour(iw5::mp::UI_DrawBuildNumber, UI_DrawBuildNumber_Hook);
    UI_DrawBuildNumber_Detour.Install();
}

Branding::~Branding()
{
    UI_DrawBuildNumber_Detour.Remove();
}
