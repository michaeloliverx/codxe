#include "patches.h"
#include "common.h"

namespace iw4
{
namespace mp
{
void EnableBouncing()
{
    // Re-enable bouncing
    // https://xoxor4d.github.io/research/mw2-bounce/
    // PM_StepSlideMove
    // .text:8210CB70                 cmpwi     cr6, r23, 0
    // .text:8210CB74                 bne       cr6, loc_8210CB24
    *(volatile uint32_t *)0x8210CB70 = 0x60000000;
    *(volatile uint32_t *)0x8210CB74 = 0x60000000;
}

void DisableIdleGunSway()
{
    // No weapon sway when aiming down sights
    // .text:821185D4                 bl        BG_CalculateViewMovement_Angles_Idle
    *(volatile uint32_t *)0x821185D4 = 0x60000000;

    // .text:82117B90                 bl        BG_ComputeAndApplyWeaponMovement_IdleAngles
    *(volatile uint32_t *)0x82117B90 = 0x60000000;
}

void DisableDvarProtection()
{
    // Read-only
    // .text:8230D680                 b         __restgprlr_27
    *(volatile uint32_t *)0x8230D680 = 0x60000000;

    // Write-protected
    // .text:8230D6A8                 b         __restgprlr_27
    *(volatile uint32_t *)0x8230D6A8 = 0x60000000;

    // Cheat-protected
    // .text:8230D6EC                 b         __restgprlr_27
    *(volatile uint32_t *)0x8230D6EC = 0x60000000;
}

Detour Weapon_RocketLauncher_Fire_Detour;

gentity_s *Weapon_RocketLauncher_Fire_Hook(gentity_s *ent, unsigned int weaponIndex, double spread, weaponParms *wp,
                                           weaponParms *gunVel, struct lockonFireParms *lockParms,
                                           lockonFireParms *magicBullet)
{
    const auto result = Weapon_RocketLauncher_Fire_Detour.GetOriginal<decltype(Weapon_RocketLauncher_Fire)>()(
        ent, weaponIndex, spread, wp, gunVel, lockParms, magicBullet);

    // COD4 logic for RPG knockback
    auto client = ent->client;
    if (client)
    {
        ent->client->ps.velocity[0] = client->ps.velocity[0] - wp->forward[0] * 64.0f;
        ent->client->ps.velocity[1] = client->ps.velocity[1] - wp->forward[1] * 64.0f;
        ent->client->ps.velocity[2] = client->ps.velocity[2] - wp->forward[2] * 64.0f;
    }

    return result;
}

patches::patches()
{
    EnableBouncing();
    DisableIdleGunSway();
    DisableDvarProtection();

    Weapon_RocketLauncher_Fire_Detour = Detour(Weapon_RocketLauncher_Fire, Weapon_RocketLauncher_Fire_Hook);
    Weapon_RocketLauncher_Fire_Detour.Install();
}

patches::~patches()
{
}
} // namespace mp
} // namespace iw4
