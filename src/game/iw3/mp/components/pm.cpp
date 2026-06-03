#include "pch.h"
#include "events.h"
#include "pm.h"

namespace iw3
{
namespace mp
{
dvar_s *pm_multi_bounce = nullptr;
dvar_s *pm_pc_mp_velocity_snap = nullptr;
dvar_s *pm_fixed_fps_enable = nullptr;
dvar_s *pm_fixed_fps = nullptr;

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

Detour Pmove_Detour;
// https://github.com/kejjjjj/iw3sptool/blob/17b669233a1ad086deed867469dc9530b84c20e6/iw3sptool/bg/bg_pmove.cpp#L11
void Pmove_Hook(pmove_t *pm)
{
    if (!pm_fixed_fps_enable->current.enabled)
        return Pmove_Detour.GetOriginal<decltype(Pmove)>()(pm);

    int msec = 0;
    int cur_msec = 1000 / pm_fixed_fps->current.integer;

    pm->cmd.serverTime = ((pm->cmd.serverTime + (cur_msec < 2 ? 2 : cur_msec) - 1) / cur_msec) * cur_msec;

    int finalTime = pm->cmd.serverTime;

    if (finalTime < pm->ps->commandTime)
    {
        return; // should not happen
    }

    if (finalTime > pm->ps->commandTime + 1000)
        pm->ps->commandTime = finalTime - 1000;
    pm->numtouch = 0;

    while (pm->ps->commandTime != finalTime)
    {
        msec = finalTime - pm->ps->commandTime;

        if (msec > cur_msec)
            msec = cur_msec;

        pm->cmd.serverTime = msec + pm->ps->commandTime;
        PmoveSingle(pm);
        memcpy(&pm->oldcmd, &pm->cmd, sizeof(pm->oldcmd));
    }
}

const float colorWhiteRGBA[4] = {1.0f, 1.0f, 1.0f, 1.0f};

void DrawFixedFPS()
{
    char buff[16];
    sprintf_s(buff, "%d", pm_fixed_fps->current.integer);

    static Font_s *font = R_RegisterFont("fonts/bigDevFont");
    float x = 620 * scrPlaceFullUnsafe.scaleVirtualToFull[0];
    float y = 15 * scrPlaceFullUnsafe.scaleVirtualToFull[1];
    R_AddCmdDrawText(buff, 16, font, x, y, 1.0, 1.0, 0.0, colorWhiteRGBA, 0);
}

pm::pm()
{
    Events::OnDvarInit(
        []
        {
            pm_multi_bounce = Dvar_RegisterBool("pm_multi_bounce", false, DVAR_CODINFO, "Enable multi-bounces");

            // This allows FPS-dependent physics
            pm_pc_mp_velocity_snap =
                Dvar_RegisterBool("pm_pc_mp_velocity_snap", false, DVAR_CODINFO,
                                  "Enable PC Multiplayer style velocity snapping (round to nearest). ");

            pm_fixed_fps_enable = Dvar_RegisterBool("pm_fixed_fps_enable", false, 0, "Enable fixed FPS mode");
            pm_fixed_fps = Dvar_RegisterInt("pm_fixed_fps", 250, 0, 1000, 0, "Fixed FPS value");
        });

    Events::OnCG_DrawActive(
        []()
        {
            if (pm_fixed_fps_enable->current.enabled)
            {
                DrawFixedFPS();
            }
        });

    // Requires jump_slowdownEnable to be set to 0
    PM_FoliageSounds_Detour = Detour(PM_FoliageSounds, PM_FoliageSounds_Hook);
    PM_FoliageSounds_Detour.Install();

    Sys_SnapVector_Detour = Detour(Sys_SnapVector, Sys_SnapVector_Hook);
    Sys_SnapVector_Detour.Install();

    Pmove_Detour = Detour(Pmove, Pmove_Hook);
    Pmove_Detour.Install();
}

pm::~pm()
{
    PM_FoliageSounds_Detour.Remove();

    Sys_SnapVector_Detour.Remove();

    Pmove_Detour.Remove();
}
} // namespace mp
} // namespace iw3
