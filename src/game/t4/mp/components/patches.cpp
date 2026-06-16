#include "pch.h"
#include "patches.h"

namespace t4
{
namespace mp
{

void DisableFastfileRsaChecks()
{
    // DBX_AuthLoad_ValidateHash
    *(volatile uint32_t *)0x821D7E9C = 0x60000000;
    *(volatile uint32_t *)0x821D7F94 = 0x60000000;
    *(volatile uint32_t *)0x821D7FE4 = 0x60000000;

    // DBX_AuthLoad_ValidateSignature
    *(volatile uint32_t *)0x821D7EB4 = 0x60000000;
}

void EnableExplosiveKnockback()
{
    // sub_8220D2D0
    // Patches NO_KNOCKBACK flag check, allowing knockback regardless of flags.
    *(volatile uint32_t *)0x8220D2E8 = 0x60000000; // NOP replaces bnelr
}

void EnableRocketJumping()
{
    // Weapon_RocketLauncher_Fire
    *(volatile uint32_t *)0x8225F98C = 0x60000000;
    *(volatile uint32_t *)0x8225F990 = 0x60000000;
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
