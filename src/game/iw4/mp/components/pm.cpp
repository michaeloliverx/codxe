#include "pch.h"
#include "pm.h"
#include "events.h"

namespace iw4
{
namespace mp
{
namespace
{
dvar_t *bg_rocketJump = nullptr;
dvar_t *bg_rocketJumpScale = nullptr;

Detour Weapon_RocketLauncher_Fire_Detour;

gentity_s *Weapon_RocketLauncher_Fire_Hook(gentity_s *ent, unsigned int weaponIndex, double spread, weaponParms *wp,
                                           weaponParms *gunVel, lockonFireParms *lockParms,
                                           lockonFireParms *magicBullet, bool magicBulletEnabled)
{

    auto *result = Weapon_RocketLauncher_Fire_Detour.GetOriginal<Weapon_RocketLauncher_Fire_t>()(
        ent, weaponIndex, spread, wp, gunVel, lockParms, magicBullet, magicBulletEnabled);

    if (ent->client && bg_rocketJump->current.enabled)
    {
        const auto scale = bg_rocketJumpScale->current.value;
        ent->client->ps.velocity[0] += (0.0f - wp->forward[0]) * scale;
        ent->client->ps.velocity[1] += (0.0f - wp->forward[1]) * scale;
        ent->client->ps.velocity[2] += (0.0f - wp->forward[2]) * scale;
    }

    return result;
}

} // namespace
pm::pm()
{

    Events::OnDvarInit(
        []
        {
            bg_rocketJump = Dvar_RegisterBool("bg_rocketJump", false, DVAR_FLAG_SERVERINFO, "Enable CoD4 rocket jumps");

            bg_rocketJumpScale = Dvar_RegisterFloat("bg_rocketJumpScale", 64.0f, 1.0f, FLT_MAX, DVAR_FLAG_SERVERINFO,
                                                    "The scale applied to the pushback force of a rocket");
        });

    Weapon_RocketLauncher_Fire_Detour = Detour(Weapon_RocketLauncher_Fire, Weapon_RocketLauncher_Fire_Hook);
    Weapon_RocketLauncher_Fire_Detour.Install();
}

pm::~pm()
{
    Weapon_RocketLauncher_Fire_Detour.Remove();
}
} // namespace mp
} // namespace iw4
