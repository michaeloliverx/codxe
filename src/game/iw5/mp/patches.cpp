#include "patches.h"
#include "common.h"

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
    *(volatile uint32_t *)0x820EB470 = 0x60000000;
    *(volatile uint32_t *)0x820EB474 = 0x60000000;
}

void DisableIdleGunSway()
{
    // No weapon sway when aiming down sights
    // .text:8210468C                 bl        sub_82104228
    // BG_CalculateViewMovement_Angles_Idle
    *(volatile uint32_t *)0x8210468C = 0x60000000;

    // .text:82103CD8                 bl        sub_82102738
    // BG_ComputeAndApplyWeaponMovement_IdleAngles
    *(volatile uint32_t *)0x82103CD8 = 0x60000000;
}

void DisableJumpSlowdown()
{
    // .text:820E86CC                 bl        Jump_ApplySlowdown
    *(volatile uint32_t *)0x820E86CC = 0x60000000;

    // // .text:820E8EB8                 bl        PM_CrashLand
    // *(volatile uint32_t *)0x820E8EB8 = 0x60000000;
}

void DisableDvarWriteChecks()
{
    // .text:8232DE20                 bne       cr6, loc_8232E0F8
    *(volatile uint32_t *)0x8232DE20 = 0x60000000;

    // .text:8232DE2C                 bne       cr6, loc_8232E0F8
    *(volatile uint32_t *)0x8232DE2C = 0x60000000;
}

patches::patches()
{
    EnableBouncing();
    DisableIdleGunSway();
    DisableJumpSlowdown();
    DisableDvarWriteChecks();
}

patches::~patches()
{
}
} // namespace mp
} // namespace iw5
