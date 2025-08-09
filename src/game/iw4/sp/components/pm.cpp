#include "pm.h"
#include "common.h"

namespace iw4
{
namespace sp
{

void ApplyBounceDepatch()
{
    // PM_StepSlideMove
    // https://xoxor4d.github.io/research/mw2-bounce/
    *(volatile uint32_t *)0x8232B250 = 0x60000000;
    *(volatile uint32_t *)0x8232B254 = 0x60000000;
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

pm::pm()
{
    ApplyBounceDepatch();

    Weapon_RocketLauncher_Fire_Detour = Detour(Weapon_RocketLauncher_Fire, Weapon_RocketLauncher_Fire_Hook);
    Weapon_RocketLauncher_Fire_Detour.Install();
}

pm::~pm()
{
    Weapon_RocketLauncher_Fire_Detour.Remove();
}
} // namespace sp
} // namespace iw4
