#include "cj_tas.h"
#include "common.h"

#define ANGLE2SHORT(x) ((int)((x) * 65536 / 360) & 65535)
#define SHORT2ANGLE(x) ((x) * (360.0 / 65536))

namespace iw3
{
    namespace mp
    {
        dvar_s *cj_tas_jump_at_edge = nullptr;
        dvar_s *cj_tas_jump_on_shoot_rpg = nullptr;

        pmove_t *clone_pmove(pmove_t *pmove)
        {
            pmove_t *copy = new pmove_t(*pmove);
            copy->ps = new playerState_s(*pmove->ps);
            return copy;
        }

        void delete_pmove(pmove_t *pmove)
        {
            if (pmove)
            {
                delete pmove->ps;
                delete pmove;
            }
        }

        void JumpAtEdge(int localClientNum)
        {
            static auto com_maxfps = Dvar_FindMalleableVar("com_maxfps");

            pmove_t *pmove = &cg_pmove[0];

            clientActive_t *localClientGlobals = &(*clients)[localClientNum];
            auto *cmd = &localClientGlobals->cmds[localClientGlobals->cmdNumber & 0x7F];

            auto predicted = clone_pmove(pmove);

            predicted->cmd.serverTime = predicted->oldcmd.serverTime;
            predicted->cmd.serverTime += 1000 / com_maxfps->current.integer;
            // TODO: does this help?
            // // Convert all three angles from viewangles to cmd->angles
            // cmd->angles[0] = ANGLE2SHORT(localClientGlobals->viewangles[0]); // Pitch
            // cmd->angles[1] = ANGLE2SHORT(localClientGlobals->viewangles[1]); // Yaw
            // cmd->angles[2] = ANGLE2SHORT(localClientGlobals->viewangles[2]); // Roll

            PmoveSingle(predicted);

            // TODOL which is button for jump?
            // 1022 = On ground
            // 1023 = In air
            bool next_frame_not_on_ground = predicted->ps->groundEntityNum != 1022;
            if (next_frame_not_on_ground)
                cmd->buttons |= 1024; // JUMP

            // bool next_frame_in_air = predicted->ps->groundEntityNum == 1023;
            // if (next_frame_in_air)
            //     cmd->buttons |= 1024; // JUMP

            delete_pmove(predicted);
        }

        void JumpOnShootRPG(int localClientNum)
        {
            static auto com_maxfps = Dvar_FindMalleableVar("com_maxfps");
            static auto rpg_mp_index = BG_FindWeaponIndexForName("rpg_mp");
            pmove_t *pmove = &cg_pmove[0];

            clientActive_t *localClientGlobals = &(*clients)[localClientNum];
            auto *cmd = &localClientGlobals->cmds[localClientGlobals->cmdNumber & 0x7F];

            if (pmove->ps->weapon != rpg_mp_index)
            {
                return; // only predict if we are using RPG
            }

            pmove_t *currentPmove = clone_pmove(pmove);

            currentPmove->cmd.serverTime = currentPmove->oldcmd.serverTime;
            currentPmove->cmd.serverTime += 1000 / com_maxfps->current.integer;
            // currentPmove->cmd.angles[1] =

            PmoveSingle(currentPmove);
            bool nextFrameShotRpg = currentPmove->ps->weaponDelay <= 3 && pmove->ps->weaponDelay != 0;
            if (nextFrameShotRpg)
            {
                // CG_GameMessage(0, "RPG SHOOT");
                cmd->buttons |= 1024; // JUMP
            }

            delete_pmove(currentPmove);
        }

        void BackwardsRPG(int localClientNum)
        {
            static auto com_maxfps = Dvar_FindMalleableVar("com_maxfps");
            static auto rpg_mp_index = BG_FindWeaponIndexForName("rpg_mp");

            pmove_t *pmove = &cg_pmove[localClientNum];
            // clientActive_t *cgameGlob = &(*clients)[localClientNum];
            // auto *cmd = &cgameGlob->cmds[cgameGlob->cmdNumber & 0x7F];

            if (pmove->ps->weapon != rpg_mp_index)
            {
                return; // only predict if we are using RPG
            }

            auto next_pmove = clone_pmove(pmove);

            // Advance time
            next_pmove->cmd.serverTime = next_pmove->oldcmd.serverTime;
            next_pmove->cmd.serverTime += 1000 / 60; // Predict next frame

            // Run the prediction
            PmoveSingle(next_pmove);

            // Check if we are shooting RPG
            bool next_frame_shot_rpg = next_pmove->ps->weaponDelay <= 3 && next_pmove->ps->weaponDelay != 0;
            if (next_frame_shot_rpg)
            {
                CG_GameMessage(0, "RPG SHOOT");
                // cmd->buttons |= 1024; // JUMP
            }

            // // TODO: what is weaponstate?
            // if (next_frame_shot_rpg && pmove->ps->weaponstate != 7)
            // {
            //     auto yaw = 90.0f; // 90 degrees

            //     auto delta_angles = cgArray[localClientNum].activeSnapshots[0].ps.delta_angles;

            //     auto adjusted_yaw = yaw - delta_angles[1]; // Adjust yaw based on delta angles

            //     // Set yaw in local camera state
            //     cgameGlob->viewangles[1] = adjusted_yaw;
            //     // Set yaw in usercmd
            //     cmd->angles[1] = ANGLE2SHORT(adjusted_yaw);
            // }

            // // Clean up the prediction
            delete_pmove(next_pmove);
        }

        void LookDownOnShootRPG(int localClientNum)
        {
            static auto rpg_mp_index = BG_FindWeaponIndexForName("rpg_mp");
            pmove_t *pmove = &cg_pmove[localClientNum];

            clientActive_t *localClientGlobals = &(*clients)[localClientNum];
            // auto *cmd = &localClientGlobals->cmds[localClientGlobals->cmdNumber & 0x7F];

            if (pmove->ps->weapon != rpg_mp_index)
            {
                return; // only predict if we are using RPG
            }

            pmove_t *currentPmove = clone_pmove(pmove);

            currentPmove->cmd.serverTime = currentPmove->oldcmd.serverTime;
            currentPmove->cmd.serverTime += 1000 / 60; // 60 FPS
            // currentPmove->cmd.angles[1] = ANGLE2SHORT(doroBot->game->toCodAngles(Vec3<float>(0, angle, 0)).y);       // Fix

            PmoveSingle(currentPmove);
            bool nextFrameShotRpg = currentPmove->ps->weaponDelay <= 3 && pmove->ps->weaponDelay != 0;
            if (nextFrameShotRpg)
            {
                float delta_x = static_cast<float>(cgArray[localClientNum].activeSnapshots[0].ps.delta_angles[0]);
                // float delta_y = static_cast<float>(cgArray[localClientNum].activeSnapshots[0].ps.delta_angles[1]);

                CG_GameMessage(0, "RPG SHOOT");
                // cmd->buttons |= 1024; // JUMP
                localClientGlobals->viewangles[0] = 70 - delta_x;
                localClientGlobals->viewangles[1] = 180;
            }

            delete_pmove(currentPmove);
        }

        Detour CL_CreateNewCommands_Detour;

        void CL_CreateNewCommands_Hook(int localClientNum)
        {
            CL_CreateNewCommands_Detour.GetOriginal<decltype(CL_CreateNewCommands)>()(localClientNum);
            if (clientUIActives[localClientNum].connectionState == CA_ACTIVE)
            {

                // Jump on next frame if we are at the edge
                if (cj_tas_jump_at_edge->current.enabled)
                {
                    JumpAtEdge(localClientNum);
                }
                // // // Jump on next frame if we shoot RPG
                // // if (cj_tas_jump_on_shoot_rpg->current.enabled)
                // // {
                // JumpOnShootRPG(localClientNum);
                // // }

                // BackwardsRPG(localClientNum);
                // LookDownOnShootRPG(localClientNum);
            }
        }

        cj_tas::cj_tas()
        {
            CL_CreateNewCommands_Detour = Detour(CL_CreateNewCommands, CL_CreateNewCommands_Hook);
            CL_CreateNewCommands_Detour.Install();

            cj_tas_jump_at_edge = Dvar_RegisterBool("cj_tas_jump_at_edge", true, 0, "");
            // cj_tas_jump_on_shoot_rpg = Dvar_RegisterBool("cj_tas_jump_on_shoot_rpg", true, 0, "Jump on next frame if we shoot RPG");
        }

        cj_tas::~cj_tas()
        {
        }
    }
}
