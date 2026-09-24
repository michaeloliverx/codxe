#include "pch.h"
#include "pm.h"

// bg_removeBarriers
int MASK_PLAYER_CLIP = 0x10000;
int MASK_BARRIER_CLIP = 0x400;

namespace iw4
{
namespace mp_tu6
{
namespace
{
dvar_t *bg_rocketJump = nullptr;
dvar_t *bg_rocketJumpScale = nullptr;
dvar_t *bg_removeBarriers = nullptr; // bg_removeBarriers

Detour Weapon_RocketLauncher_Fire_Detour;
Detour PmoveSingle_Detour; // bg_removeBarriers
Detour PM_CheckLadderMove_Detour; // bg_removeBarriers

gentity_s *Weapon_RocketLauncher_Fire_Hook(gentity_s *ent, unsigned int weaponIndex, double spread, weaponParms *wp,
                                           weaponParms *gunVel, lockonFireParms *lockParms,
                                           lockonFireParms *magicBullet)
{
    const auto result = Weapon_RocketLauncher_Fire_Detour.GetOriginal<decltype(Weapon_RocketLauncher_Fire)>()(
        ent, weaponIndex, spread, wp, gunVel, lockParms, magicBullet);

    if (ent->client && bg_rocketJump && bg_rocketJumpScale && bg_rocketJump->current.enabled)
    {
        const auto scale = bg_rocketJumpScale->current.value;
        ent->client->ps.velocity[0] += (0.0f - wp->forward[0]) * scale;
        ent->client->ps.velocity[1] += (0.0f - wp->forward[1]) * scale;
        ent->client->ps.velocity[2] += (0.0f - wp->forward[2]) * scale;
    }

    return result;
}

// bg_removeBarriers
void PmoveSingle_Hook(pmove_t* pm)
{
    if (bg_removeBarriers && bg_removeBarriers->current.enabled)
    {
        if (pm != nullptr && (pm->ps->pm_flags & PMF_LADDER) == 0)
        {
            pm->tracemask &= ~MASK_PLAYER_CLIP;
            pm->tracemask |= MASK_BARRIER_CLIP;
        }
    }

    PmoveSingle_Detour.GetOriginal<decltype(PmoveSingle)>()(pm);
}

void PM_CheckLadderMove_Hook(pmove_t* pm, pml_t* pml)
{
    const auto fix_ladder_movement = (bg_removeBarriers && bg_removeBarriers->current.enabled && pm != nullptr);

    if (fix_ladder_movement)
    {
        pm->tracemask |= MASK_PLAYER_CLIP;
    }

    PM_CheckLadderMove_Detour.GetOriginal<decltype(PM_CheckLadderMove)>()(pm, pml);

    if (fix_ladder_movement && (pm->ps->pm_flags & PMF_LADDER) == 0)
    {
        pm->tracemask &= ~MASK_PLAYER_CLIP;
    }
}

} // namespace

void pm::OnDvarInit()
{
    bg_rocketJump = Dvar_RegisterBool("bg_rocketJump", false, DVAR_FLAG_SERVERINFO, "Enable CoD4 rocket jumps");
    bg_rocketJumpScale = Dvar_RegisterFloat("bg_rocketJumpScale", 64.0f, 1.0f, FLT_MAX, DVAR_FLAG_SERVERINFO,
                                            "The scale applied to the pushback force of a rocket");
    // bg_removeBarriers
    bg_removeBarriers = Dvar_RegisterBool("bg_removeBarriers", false, DVAR_CODINFO, "Remove player collision with out of bound barriers");
}

pm::pm()
{
    Weapon_RocketLauncher_Fire_Detour = Detour(Weapon_RocketLauncher_Fire, Weapon_RocketLauncher_Fire_Hook);
    Weapon_RocketLauncher_Fire_Detour.Install();
    // bg_removeBarriers
    PmoveSingle_Detour = Detour(PmoveSingle, PmoveSingle_Hook);
    PmoveSingle_Detour.Install();

    PM_CheckLadderMove_Detour = Detour(PM_CheckLadderMove, PM_CheckLadderMove_Hook);
    PM_CheckLadderMove_Detour.Install();
}

pm::~pm()
{
    Weapon_RocketLauncher_Fire_Detour.Remove();
    // bg_removeBarriers
    PmoveSingle_Detour.Remove();
    PM_CheckLadderMove_Detour.Remove();
}
} // namespace mp_tu6
} // namespace iw4
