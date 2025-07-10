#include "cg.h"
#include "common.h"

namespace t4
{
    namespace mp
    {
        dvar_s *bg_bobIdle = nullptr;

        Detour BG_CalculateWeaponPosition_IdleAngles_Detour;

        void BG_CalculateWeaponPosition_IdleAngles_Hook(weaponState_t *ws, float *angles)
        {
            if (!bg_bobIdle->current.enabled)
            {
                return;
            }
            BG_CalculateWeaponPosition_IdleAngles_Detour.GetOriginal<decltype(BG_CalculateWeaponPosition_IdleAngles)>()(ws, angles);
        }

        Detour BG_CalculateView_IdleAngles_Detour;

        void BG_CalculateView_IdleAngles_Hook(viewState_t *vs, float *angles)
        {
            if (!bg_bobIdle->current.enabled)
            {
                return;
            }
            BG_CalculateView_IdleAngles_Detour.GetOriginal<decltype(BG_CalculateView_IdleAngles)>()(vs, angles);
        }

        Detour Menus_OpenByName_Detour;

        void Menus_OpenByName_Hook(UiContext *dc, const char *menuName)
        {
            // Splitscreen map list only contains a subset of maps
            // The menu UI determines the map list based on the splitscreen setting
            if (strcmp(menuName, "menu_gamesetup_splitscreen") == 0)
                Cbuf_ExecuteBuffer(0, 0, "set splitscreen 0");

            Menus_OpenByName_Detour.GetOriginal<decltype(Menus_OpenByName)>()(dc, menuName);
        }

        cg::cg()
        {
            DbgPrint("cg initialized\n");
            bg_bobIdle = Dvar_RegisterBool("bg_bobIdle", true, DVAR_FLAG_NONE, "Idle gun sway");

            BG_CalculateWeaponPosition_IdleAngles_Detour = Detour(BG_CalculateWeaponPosition_IdleAngles, BG_CalculateWeaponPosition_IdleAngles_Hook);
            BG_CalculateWeaponPosition_IdleAngles_Detour.Install();

            BG_CalculateView_IdleAngles_Detour = Detour(BG_CalculateView_IdleAngles, BG_CalculateView_IdleAngles_Hook);
            BG_CalculateView_IdleAngles_Detour.Install();

            Menus_OpenByName_Detour = Detour(Menus_OpenByName, Menus_OpenByName_Hook);
            Menus_OpenByName_Detour.Install();
        }

        cg::~cg()
        {
            DbgPrint("cg shutdown\n");
            BG_CalculateWeaponPosition_IdleAngles_Detour.Remove();
            BG_CalculateView_IdleAngles_Detour.Remove();

            Menus_OpenByName_Detour.Remove();
        }
    }
}
