#include "cg.h"
#include "cj_tas.h"
#include "common.h"

namespace iw3
{
    namespace mp
    {
        dvar_s *bg_bobIdle = nullptr;

        dvar_s *cg_scoreboardLabel_Score = nullptr;
        dvar_s *cg_scoreboardLabel_Kills = nullptr;
        dvar_s *cg_scoreboardLabel_Assists = nullptr;
        dvar_s *cg_scoreboardLabel_Deaths = nullptr;

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

        Detour UI_SafeTranslateString_Detour;

        const char *UI_SafeTranslateString_Hook(char *reference)
        {
            if (_ReturnAddress() == (void *)0x822FDDDC) // CG_DrawScoreboard_ListColumnHeaders -> UI_SafeTranslateString
            {
                if (strcmp(reference, "CGAME_SB_SCORE") == 0)
                {
                    const auto val = Dvar_GetString("cg_scoreboardLabel_Score");
                    if (val && *val)
                        return val;
                }
                else if (strcmp(reference, "CGAME_SB_KILLS") == 0)
                {
                    const char *val = Dvar_GetString("cg_scoreboardLabel_Kills");
                    if (val && *val)
                        return val;
                }
                else if (strcmp(reference, "CGAME_SB_ASSISTS") == 0)
                {
                    const char *val = Dvar_GetString("cg_scoreboardLabel_Assists");
                    if (val && *val)
                        return val;
                }
                else if (strcmp(reference, "CGAME_SB_DEATHS") == 0)
                {
                    const char *val = Dvar_GetString("cg_scoreboardLabel_Deaths");
                    if (val && *val)
                        return val;
                }
            }

            return UI_SafeTranslateString_Detour.GetOriginal<decltype(UI_SafeTranslateString)>()(reference);
        }

        Detour Menus_OpenByName_Detour;

        void Menus_OpenByName_Hook(UiContext *dc, const char *menuName)
        {
            if (strcmp(menuName, "settings_map_splitscreen") == 0)
                // Splitscreen map list only contains a subset of maps
                // Opening the full map settings menu allows selecting any map
                Menus_OpenByName_Detour.GetOriginal<decltype(Menus_OpenByName)>()(dc, "settings_map");
            else
                Menus_OpenByName_Detour.GetOriginal<decltype(Menus_OpenByName)>()(dc, menuName);
        }

        Detour CG_Init_Detour;

        void CG_Init_Hook(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum)
        {
            CG_Init_Detour.GetOriginal<decltype(CG_Init)>()(localClientNum, serverMessageNum, serverCommandSequence, clientNum);
            cj_tas::On_CG_Init();
        }

        cg::cg()
        {
            Menus_OpenByName_Detour = Detour(Menus_OpenByName, Menus_OpenByName_Hook);
            Menus_OpenByName_Detour.Install();

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

            UI_SafeTranslateString_Detour = Detour(UI_SafeTranslateString, UI_SafeTranslateString_Hook);
            UI_SafeTranslateString_Detour.Install();

            CG_Init_Detour = Detour(CG_Init, CG_Init_Hook);
            CG_Init_Detour.Install();

            cg_scoreboardLabel_Score = Dvar_RegisterString(
                "cg_scoreboardLabel_Score",
                "",
                DVAR_FLAG_NONE,
                "Override label for 'Score' column on scoreboard");

            cg_scoreboardLabel_Kills = Dvar_RegisterString(
                "cg_scoreboardLabel_Kills",
                "",
                DVAR_FLAG_NONE,
                "Override label for 'Kills' column on scoreboard");

            cg_scoreboardLabel_Assists = Dvar_RegisterString(
                "cg_scoreboardLabel_Assists",
                "",
                DVAR_FLAG_NONE,
                "Override label for 'Assists' column on scoreboard");

            cg_scoreboardLabel_Deaths = Dvar_RegisterString(
                "cg_scoreboardLabel_Deaths",
                "",
                DVAR_FLAG_NONE,
                "Override label for 'Deaths' column on scoreboard");
        }

        cg::~cg()
        {
            Menus_OpenByName_Detour.Remove();
            UI_DrawBuildNumber_Detour.Remove();
            UI_SafeTranslateString_Detour.Remove();
            BG_CalculateWeaponPosition_IdleAngles_Detour.Remove();
            BG_CalculateView_IdleAngles_Detour.Remove();
            CG_Init_Detour.Remove();
        }
    }
}
