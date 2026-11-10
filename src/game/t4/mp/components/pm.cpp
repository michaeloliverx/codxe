#include "pch.h"
#include "pm.h"

#include <cmath>

#define PM_NOCLIP 2
#define PM_UFO 3

namespace t4
{
namespace mp
{
namespace
{
Detour PmoveSingle_Detour;
Detour CL_AddViewAngles_Detour;

static const float fullRotation = 360.0f;
static const float halfRotation = 180.0f;

float WrapViewAngleCorrection(float angle)
{
    if (angle >= -halfRotation && angle <= halfRotation)
        return angle;

    angle = fmodf(angle, fullRotation);
    if (angle > halfRotation)
        angle -= fullRotation;
    else if (angle < -halfRotation)
        angle += fullRotation;
    return angle;
}

void NormalizeViewAngleCorrection(float *correction)
{
    for (int axis = 0; axis < 3; ++axis)
        correction[axis] = WrapViewAngleCorrection(correction[axis]);
}

int CL_AddViewAngles_Hook(int localClientNum, float *correction)
{
    // Fix choppy up/down aiming after holding the stick at a look limit.
    // Prediction feeds the pitch clamp's delta-angle change back into client viewangles, then reuses this
    // same array to adjust queued commands. Wrap it in place so both receive the equivalent small correction;
    // otherwise full rotations accumulate until small stick movements lose float precision.
    NormalizeViewAngleCorrection(correction);
    return CL_AddViewAngles_Detour.GetOriginal<decltype(CL_AddViewAngles)>()(localClientNum, correction);
}

void PmoveSingle_Hook(pmove_t *pm)
{
    if (pm->ps->pm_type == PM_UFO)
    {
        // Fix UFO prediction desync: this mode skips the walking history update, so client and server can retain
        // different old velocities. Clear them before stock movement to prevent different inertia calculations.
        pm->ps->oldVelocity[0] = 0.0f;
        pm->ps->oldVelocity[1] = 0.0f;
    }

    PmoveSingle_Detour.GetOriginal<decltype(PmoveSingle)>()(pm);
}
} // namespace

pm::pm()
{
    PmoveSingle_Detour = Detour(PmoveSingle, PmoveSingle_Hook);
    PmoveSingle_Detour.Install();

    CL_AddViewAngles_Detour = Detour(CL_AddViewAngles, CL_AddViewAngles_Hook);
    CL_AddViewAngles_Detour.Install();
}

pm::~pm()
{
    CL_AddViewAngles_Detour.Remove();
    PmoveSingle_Detour.Remove();
}
} // namespace mp
} // namespace t4
