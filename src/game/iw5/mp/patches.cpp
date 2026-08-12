#include "pch.h"
#include "patches.h"

namespace iw5
{
namespace mp
{
void EnableBouncing()
{
    // Re-enable bouncing
    // https://xoxor4d.github.io/research/mw2-bounce/
    // .text:820EB470                 cmpwi     cr6, r23, 0
    // .text:820EB474                 bne       cr6, loc_820EB424
    ppc::Nop(0x820EB470);
    ppc::Nop(0x820EB474);
}

void DisableIdleGunSway()
{
    // No weapon sway when aiming down sights
    // .text:8210468C                 bl        sub_82104228
    // BG_CalculateViewMovement_Angles_Idle
    ppc::Nop(0x8210468C);

    // .text:82103CD8                 bl        sub_82102738
    // BG_ComputeAndApplyWeaponMovement_IdleAngles
    ppc::Nop(0x82103CD8);
}

void DisableJumpSlowdown()
{
    // .text:820E86CC                 bl        Jump_ApplySlowdown
    ppc::Nop(0x820E86CC);

    // // .text:820E8EB8                 bl        PM_CrashLand
    // ppc::Nop(0x820E8EB8);
}

void DisableDvarWriteChecks()
{
    // .text:8232DE20                 bne       cr6, loc_8232E0F8
    ppc::Nop(0x8232DE20);

    // .text:8232DE2C                 bne       cr6, loc_8232E0F8
    ppc::Nop(0x8232DE2C);
}

Detour Jump_Start_Detour;
Detour CL_ConsolePrint_Detour;

void CL_ConsolePrint_Hook(LocalClientNum_t localClientNum, int channel, const char *txt, unsigned int duration,
                          unsigned int pixelWidth, int flags)
{
    DbgPrint("[codxe][IW5][CL_ConsolePrint] %s", txt ? txt : "<null>");
    CL_ConsolePrint_Detour.GetOriginal<CL_ConsolePrint_t>()(localClientNum, channel, txt, duration, pixelWidth, flags);
}

void Jump_Start_Hook(pmove_t *pm, pml_t *pml, double height)
{
    static const dvar_t *jump_height = Dvar_FindMalleableVar("jump_height");
    Jump_Start_Detour.GetOriginal<Jump_Start_t>()(pm, pml, jump_height->current.value);
}

patches::patches()
{
    // EnableBouncing();
    // DisableIdleGunSway();
    // DisableJumpSlowdown();

    Jump_Start_Detour = Detour(Jump_Start, Jump_Start_Hook);
    // Jump_Start_Detour.Install();

    CL_ConsolePrint_Detour = Detour(CL_ConsolePrint, CL_ConsolePrint_Hook);
    CL_ConsolePrint_Detour.Install();
}

patches::~patches()
{
    CL_ConsolePrint_Detour.Remove();
    // Jump_Start_Detour.Remove();
}
} // namespace mp
} // namespace iw5
