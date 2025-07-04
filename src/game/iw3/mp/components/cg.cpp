#include "cg.h"
#include "common.h"

namespace iw3
{
    namespace mp
    {
        dvar_s *bg_bobIdle = nullptr;

        Detour BG_CalculateWeaponPosition_IdleAngles_Detour;

        void BG_CalculateWeaponPosition_IdleAngles_Hook(weaponState_t *ws, float *angles)
        {
            if (!bg_bobIdle->current.enabled)
                return;

            BG_CalculateWeaponPosition_IdleAngles_Detour.GetOriginal<decltype(BG_CalculateWeaponPosition_IdleAngles)>()(ws, angles);
        }

        Detour BG_CalculateView_IdleAngles_Detour;

        void BG_CalculateView_IdleAngles_Hook(viewState_t *vs, float *angles)
        {
            if (!bg_bobIdle->current.enabled)
                return;

            BG_CalculateView_IdleAngles_Detour.GetOriginal<decltype(BG_CalculateView_IdleAngles)>()(vs, angles);
        }

        Detour R_DrawAllDynEnt_Detour;

        void R_DrawAllDynEnt_Hook(const GfxViewInfo *viewInfo)
        {
            if (Dvar_GetBool("r_drawDynEnts"))
                R_DrawAllDynEnt_Detour.GetOriginal<decltype(R_DrawAllDynEnt)>()(viewInfo);
        }

        void DrawBranding()
        {
            const char *branding = "IW3xe";
            const char *build = __DATE__ " " __TIME__;
            char brandingWithBuild[256];

            // Combine branding and build number
            _snprintf(brandingWithBuild, sizeof(brandingWithBuild), "%s (Build %d)", branding, BUILD_NUMBER);

            static Font_s *font = (Font_s *)R_RegisterFont("fonts/consoleFont");
            float color[4] = {1.0, 1.0, 1.0, 0.4};

            R_AddCmdDrawText(brandingWithBuild, 256, font, 10, 20, 1.0, 1.0, 0.0, color, 0);
            R_AddCmdDrawText(build, 256, font, 10, 40, 1.0, 1.0, 0.0, color, 0);
        }

        Detour UI_DrawBuildNumber_Detour;

        void UI_DrawBuildNumber_Hook(const int localClientNum)
        {
            DrawBranding();
            // Omit the original build number drawing
            // UI_DrawBuildNumber_Detour.GetOriginal<decltype(UI_DrawBuildNumber)>()
        }

        cg::cg()
        {
            UI_DrawBuildNumber_Detour = Detour(UI_DrawBuildNumber, UI_DrawBuildNumber_Hook);
            UI_DrawBuildNumber_Detour.Install();

            // Default to true for idle gun sway
            // This is the default behavior in the original game.
            bg_bobIdle = Dvar_RegisterBool("bg_bobIdle", true, 0, "Idle gun sway");

            BG_CalculateWeaponPosition_IdleAngles_Detour = Detour(BG_CalculateWeaponPosition_IdleAngles, BG_CalculateWeaponPosition_IdleAngles_Hook);
            BG_CalculateWeaponPosition_IdleAngles_Detour.Install();

            BG_CalculateView_IdleAngles_Detour = Detour(BG_CalculateView_IdleAngles, BG_CalculateView_IdleAngles_Hook);
            BG_CalculateView_IdleAngles_Detour.Install();

            R_DrawAllDynEnt_Detour = Detour(R_DrawAllDynEnt, R_DrawAllDynEnt_Hook);
            R_DrawAllDynEnt_Detour.Install();

            Dvar_RegisterBool("r_drawDynEnts", true, 0, "Draw dynamic entities");
        }

        cg::~cg()
        {
            UI_DrawBuildNumber_Detour.Remove();

            BG_CalculateWeaponPosition_IdleAngles_Detour.Remove();
            BG_CalculateView_IdleAngles_Detour.Remove();
        }
    }
}
