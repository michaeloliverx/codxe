#include "pm.h"
#include "common.h"

namespace iw3
{
    namespace mp
    {
        dvar_s *pm_multi_bounce = nullptr;
        dvar_s *pm_pc_mp_velocity_snap = nullptr;

        Detour Sys_SnapVector_Detour;

        void Sys_SnapVector_Hook(float *v)
        {
            if (pm_pc_mp_velocity_snap->current.enabled)
            {
                // Use __frnd for round-to-nearest-even behavior
                v[0] = (float)__frnd((double)v[0]);
                v[1] = (float)__frnd((double)v[1]);
                v[2] = (float)__frnd((double)v[2]);
            }
            else
            {
                Sys_SnapVector_Detour.GetOriginal<decltype(Sys_SnapVector)>()(v);
            }
        }

        Detour PM_FoliageSounds_Detour;

        void PM_FoliageSounds_Hook(pmove_t *pm)
        {
            PM_FoliageSounds_Detour.GetOriginal<decltype(PM_FoliageSounds)>()(pm);

            // https://github.com/kejjjjj/iw3sptool/blob/17b669233a1ad086deed867469dc9530b84c20e6/iw3sptool/bg/bg_pmove.cpp#L106-L124
            if (pm_multi_bounce->current.enabled)
            {
                static float previousZ = pm->ps->jumpOriginZ;

                if (pm->ps->jumpOriginZ != NULL)
                    previousZ = pm->ps->jumpOriginZ;

                pm->ps->pm_flags = pm->ps->pm_flags & 0xFFFFFE7F | 0x4000;
                pm->ps->jumpOriginZ = previousZ;
            }
        }

        pm::pm()
        {
            pm_multi_bounce = Dvar_RegisterBool("pm_multi_bounce", false, DVAR_CODINFO, "Enable multi-bounces");

            // This allows FPS-dependent physics
            pm_pc_mp_velocity_snap = Dvar_RegisterBool(
                "pm_pc_mp_velocity_snap",
                false,
                DVAR_CODINFO,
                "Enable PC Multiplayer style velocity snapping (round to nearest). ");

            // Requires jump_slowdownEnable to be set to 0
            PM_FoliageSounds_Detour = Detour(PM_FoliageSounds, PM_FoliageSounds_Hook);
            PM_FoliageSounds_Detour.Install();

            Sys_SnapVector_Detour = Detour(Sys_SnapVector, Sys_SnapVector_Hook);
            Sys_SnapVector_Detour.Install();
        }

        pm::~pm()
        {
            PM_FoliageSounds_Detour.Remove();
            Sys_SnapVector_Detour.Remove();
        }
    }
}
