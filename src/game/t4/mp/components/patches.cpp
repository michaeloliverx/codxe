#include "pch.h"
#include "patches.h"

namespace t4
{
namespace mp
{
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
    EnableExplosiveKnockback();
    EnableRocketJumping();
}
} // namespace mp
} // namespace t4
