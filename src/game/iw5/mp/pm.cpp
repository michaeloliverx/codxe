#include "pm.h"
#include "common.h"

namespace iw5
{
namespace mp
{

Detour Weapon_RocketLauncher_Fire_Detour;

gentity_s *Weapon_RocketLauncher_Fire_Hook(gentity_s *ent, const Weapon *weapon, double spread, weaponParms *wp,
                                           weaponParms *gunVel, missileFireParms *fireParms,
                                           missileFireParms *magicBullet, bool a8)
{
    // NOTE: remove this hack and remove shellshock another way
    // // Call the original function
    // gentity_s *result = Weapon_RocketLauncher_Fire_Detour.GetOriginal<Weapon_RocketLauncher_Fire_t>()(
    //     ent, weapon, spread, wp, gunVel, fireParms, magicBullet, a8);

    // Reimplement COD4 logic for RPG knockback
    // Apply at the end which matches the original game behavior
    if (ent->client)
    {
        ent->client->ps.velocity[0] = ent->client->ps.velocity[0] - wp->forward[0] * 64.0f;
        ent->client->ps.velocity[1] = ent->client->ps.velocity[1] - wp->forward[1] * 64.0f;
        ent->client->ps.velocity[2] = ent->client->ps.velocity[2] - wp->forward[2] * 64.0f;
    }

    return 0;
}

PlayerMovement::PlayerMovement()
{
    Weapon_RocketLauncher_Fire_Detour = Detour(Weapon_RocketLauncher_Fire, Weapon_RocketLauncher_Fire_Hook);
    Weapon_RocketLauncher_Fire_Detour.Install();
}

PlayerMovement::~PlayerMovement()
{
    Weapon_RocketLauncher_Fire_Detour.Remove();
}
} // namespace mp
} // namespace iw5
