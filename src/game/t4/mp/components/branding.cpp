#include "branding.h"
#include "common.h"

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
            const char *branding = "CoDxe T3 MP";
            const char *build = __DATE__ " " __TIME__;
            char brandingWithBuild[256];

            _snprintf(brandingWithBuild, sizeof(brandingWithBuild), "%s (Build %d)", branding, BUILD_NUMBER);

            static Font_s *font = (Font_s *)DB_FindXAssetHeader(ASSET_TYPE_FONT, "fonts/consoleFont", 1, -1);

            static float color[4] = {1.0, 1.0, 1.0, 0.6};
            R_AddCmdDrawText(brandingWithBuild, 256, font, 10, 20, 1, 1, 1, color, 0);
            R_AddCmdDrawText(build, 256, font, 10, 40, 1.0, 1.0, 0.0, color, 0);
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
    }
}
