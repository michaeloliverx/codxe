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

        cg::cg()
        {
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
            BG_CalculateWeaponPosition_IdleAngles_Detour.Remove();
            BG_CalculateView_IdleAngles_Detour.Remove();
        }
    }
}
