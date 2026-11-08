#include "pch.h"
#include "branding.h"
#include "mods.h"

namespace t4
{
namespace mp
{
Detour UI_DrawBuildNumber_Detour;

void Branding::UI_DrawBuildNumber_Hook(const int localClientNum)
{
    DrawBranding();
}

void Branding::DrawBranding()
{
    const std::string activeMod = ModList::GetActiveName();
    const char *brandingWithBuild = branding::GetBrandingString(activeMod.c_str());

    static Font_s *font = (Font_s *)DB_FindXAssetHeader(ASSET_TYPE_FONT, "fonts/consoleFont", 1, -1);

    static float color[4] = {1.0, 1.0, 1.0, 0.6};
    R_AddCmdDrawText(brandingWithBuild, 256, font, 10, 20, 1.0f, 1.0f, 0.0f, color, 0);
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
} // namespace t4
