#include "pch.h"
#include "patches.h"

namespace t4
{
namespace mp
{

void DisableFastfileRsaChecks()
{
    // DBX_AuthLoad_ValidateHash
    ppc::Nop(0x821D7E9C);
    ppc::Nop(0x821D7F94);
    ppc::Nop(0x821D7FE4);

    // DBX_AuthLoad_ValidateSignature
    ppc::Nop(0x821D7EB4);
}

void EnableExplosiveKnockback()
{
    // sub_8220D2D0
    // Patches NO_KNOCKBACK flag check, allowing knockback regardless of flags.
    ppc::Nop(0x8220D2E8); // NOP replaces bnelr
}

void EnableRocketJumping()
{
    // Weapon_RocketLauncher_Fire
    ppc::Nop(0x8225F98C);
    ppc::Nop(0x8225F990);
}

Patches::Patches()
{
    DisableFastfileRsaChecks();
    EnableExplosiveKnockback();
    EnableRocketJumping();

    // default loc_warnings off to prevent console spam
    *(volatile uint8_t *)0x82292ABF = 0x0;
}
} // namespace mp
} // namespace t4
