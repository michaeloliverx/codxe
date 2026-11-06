#include "pch.h"
#include "pm.h"

#define PM_NOCLIP 2
#define PM_UFO 3

namespace t4
{
namespace mp
{
namespace
{
Detour PmoveSingle_Detour;

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
}

pm::~pm()
{
    PmoveSingle_Detour.Remove();
}
} // namespace mp
} // namespace t4
