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

        cg::cg()
        {
            DbgPrint("cg initialized\n");
            bg_bobIdle = Dvar_RegisterBool("bg_bobIdle", true, DVAR_FLAG_NONE, "Idle gun sway");

            BG_CalculateWeaponPosition_IdleAngles_Detour = Detour(BG_CalculateWeaponPosition_IdleAngles, BG_CalculateWeaponPosition_IdleAngles_Hook);
            BG_CalculateWeaponPosition_IdleAngles_Detour.Install();

            BG_CalculateView_IdleAngles_Detour = Detour(BG_CalculateView_IdleAngles, BG_CalculateView_IdleAngles_Hook);
            BG_CalculateView_IdleAngles_Detour.Install();
        }

        cg::~cg()
        {
            DbgPrint("cg shutdown\n");
            BG_CalculateWeaponPosition_IdleAngles_Detour.Remove();
            BG_CalculateView_IdleAngles_Detour.Remove();
        }
    }
}
