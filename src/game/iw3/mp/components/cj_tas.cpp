#include "cj_tas.h"
#include "common.h"

#define ANGLE2SHORT(x) ((int)((x) * 65536 / 360) & 65535)
// #define SHORT2ANGLE(x) ((x) * (360.0 / 65536))
#define SHORT2ANGLE(x) ((x) * (360.0f / 65536.0f))

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

        // pmove_t *predict_pmove(int localClientNum)
        // {
        //     // On console this is 85!
        //     // vsync is 60 FPS, so we predict at 60 FPS
        //     // static auto com_maxfps = Dvar_FindMalleableVar("com_maxfps");

        //     // Current in-flight pmove
        //     pmove_t *pmove_current = &cg_pmove[localClientNum];
        //     auto pmove_current_clone = clone_pmove(pmove_current);

        //     // Advance time for the current pmove by one frame
        //     pmove_current_clone->cmd.serverTime = pmove_current_clone->oldcmd.serverTime;
        //     pmove_current_clone->cmd.serverTime += 1000 / 60;

        //     // auto cg = &(*cgArray)[localClientNum];
        //     auto ca = &(*clients)[localClientNum];

        //     // DO I NEED DELTA ANGLES HERE?
        //     pmove_current_clone->cmd.angles[PITCH] = ANGLE2SHORT(ca->viewangles[PITCH]);
        //     pmove_current_clone->cmd.angles[YAW] = ANGLE2SHORT(ca->viewangles[YAW]);

        //     // Run the prediction
        //     PmoveSingle(pmove_current_clone);

        //     return pmove_current_clone;
        // }

        pmove_t *predict_pmove(int localClientNum, int framesToAdvance = 1)
        {
            pmove_t *pmove_current = &cg_pmove[localClientNum];
            auto pmove_clone = clone_pmove(pmove_current);

            auto ca = &(*clients)[localClientNum];

            int frameTime = 1000 / 60; // assuming 60 FPS
            for (int i = 0; i < framesToAdvance; ++i)
            {
                // Advance time by 1 frame
                pmove_clone->oldcmd = pmove_clone->cmd;
                pmove_clone->cmd.serverTime += frameTime;

                // Set input angles (you can vary these per frame if needed)
                pmove_clone->cmd.angles[PITCH] = ANGLE2SHORT(ca->viewangles[PITCH]);
                pmove_clone->cmd.angles[YAW] = ANGLE2SHORT(ca->viewangles[YAW]);

                // Run single frame of prediction
                PmoveSingle(pmove_clone);
            }

            return pmove_clone;
        }

        void JumpAtEdge(int localClientNum)
        {
            clientActive_t *localClientGlobals = &(*clients)[localClientNum];
            auto *current_cmd = &localClientGlobals->cmds[localClientGlobals->cmdNumber & 0x7F];

            pmove_t *pmove_current = &cg_pmove[localClientNum];
            auto pmove_predicted = predict_pmove(localClientNum);

            // TODOL which is button for jump?
            // 1022 = On ground
            // 1023 = In air

            bool on_ground = pmove_current->ps->groundEntityNum == 1022;
            bool in_air_next_frame = pmove_predicted->ps->groundEntityNum == 1023;
            // bool in_air = pmove_current->ps->groundEntityNum == 1023;

            // bool in_air = pmove_current->ps->groundEntityNum == 1023;

            if (on_ground && in_air_next_frame)
                current_cmd->buttons |= 1024; // JUMP

            // bool next_frame_in_air = predicted->ps->groundEntityNum == 1023;
            // if (next_frame_in_air)
            //     cmd->buttons |= 1024; // JUMP

            delete_pmove(pmove_predicted);
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

        // void LookdownRPG(int localClientNum)
        // {
        //     // Default value of "player_view_pitch_down" on console
        //     auto rpg_lookdown_pitch = 70.0f;
        //     // auto rpg_lookdown_yaw = 180.0f;

        //     pmove_t *pmove_current = &cg_pmove[localClientNum];
        //     pmove_t *pmove_predicted = predict_pmove(localClientNum);

        //     clientActive_t *cgameGlob = &(*clients)[localClientNum];
        //     usercmd_s *cmd = &cgameGlob->cmds[cgameGlob->cmdNumber & 0x7F];

        //     // Check if we are holding RPG

        //     static auto should_reset = false;
        //     static auto previous_pitch = cgameGlob->viewangles[PITCH];
        //     // static auto previous_yaw = cgameGlob->viewangles[YAW];

        //     if (should_reset)
        //     {
        //         // Reset the view angles to the previous state
        //         cgameGlob->viewangles[PITCH] = previous_pitch;
        //         // cgameGlob->viewangles[YAW] = previous_yaw;
        //         should_reset = false;
        //     }

        //     static auto rpg_mp_index = BG_FindWeaponIndexForName("rpg_mp");
        //     auto holding_rpg = pmove_current->ps->weapon == rpg_mp_index;
        //     auto reloading = pmove_current->ps->weaponstate == WEAPON_RELOADING;

        //     bool shoot_rpg_next_frame = pmove_predicted->ps->weaponDelay <= 3 && pmove_predicted->ps->weaponDelay != 0;

        //     if (shoot_rpg_next_frame && holding_rpg && !reloading)
        //     {
        //         previous_pitch = cgameGlob->viewangles[PITCH];
        //         // previous_yaw = cgameGlob->viewangles[YAW];

        //         auto &delta_angles = cgArray[localClientNum].activeSnapshots[0].ps.delta_angles;
        //         cmd->angles[PITCH] = ANGLE2SHORT(rpg_lookdown_pitch - delta_angles[PITCH]);

        //         // should_reset = true;
        //     }

        //     // Clean up the prediction
        //     delete_pmove(pmove_predicted);
        // }

        // void LookdownRPG(int localClientNum)
        // {
        //     // Default value of "player_view_pitch_down" on console
        //     auto rpg_lookdown_pitch = 70.0f;

        //     pmove_t *pmove_current = &cg_pmove[localClientNum];
        //     pmove_t *pmove_predicted = predict_pmove(localClientNum);

        //     clientActive_t *cgameGlob = &(*clients)[localClientNum];
        //     usercmd_s *cmd = &cgameGlob->cmds[cgameGlob->cmdNumber & 0x7F];

        //     static auto should_reset = false;
        //     static auto previous_pitch = cgameGlob->viewangles[PITCH];

        //     if (should_reset)
        //     {
        //         // Reset the view angles to the previous state
        //         cgameGlob->viewangles[PITCH] = previous_pitch;
        //         should_reset = false;
        //     }

        //     static auto rpg_mp_index = BG_FindWeaponIndexForName("rpg_mp");
        //     auto holding_rpg = pmove_current->ps->weapon == rpg_mp_index;
        //     auto reloading = pmove_current->ps->weaponstate == WEAPON_RELOADING;

        //     bool shot_rpg_next_frame = pmove_predicted->ps->weaponDelay <= 3 && pmove_predicted->ps->weaponDelay != 0;

        //     if (shot_rpg_next_frame && holding_rpg && !reloading)
        //     {
        //         CG_GameMessage(0, "RPG SHOOT PREDICTED");
        //         previous_pitch = cgameGlob->viewangles[PITCH];

        //         // auto &delta_angles = cgArray[localClientNum].activeSnapshots[0].ps.delta_angles;
        //         // cmd->angles[PITCH] = ANGLE2SHORT(rpg_lookdown_pitch - delta_angles[PITCH]);

        //         should_reset = true;
        //     }

        //     // Clean up the prediction
        //     delete_pmove(pmove_predicted);
        // }

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
                // float delta_x = static_cast<float>(cgArray[localClientNum].activeSnapshots[0].ps.delta_angles[0]);
                // float delta_y = static_cast<float>(cgArray[localClientNum].activeSnapshots[0].ps.delta_angles[1]);

                CG_GameMessage(0, "RPG SHOOT");
                // cmd->buttons |= 1024; // JUMP
                // localClientGlobals->viewangles[0] = 70 - delta_x;
                localClientGlobals->viewangles[1] = 180;
            }

            delete_pmove(currentPmove);
        }

        // #define ANGLE2SHORT(x) ((int)((x) * 65536 / 360) & 65535)
        // #define SHORT2ANGLE(x) ((x) * (360.0f / 65536))

        //         pmove_t *predict_pmove(int localClientNum)
        //         {
        //             static auto com_maxfps = Dvar_FindMalleableVar("com_maxfps");

        //             pmove_t *pmove_current = &cg_pmove[localClientNum];
        //             auto pmove_current_clone = clone_pmove(pmove_current);

        //             // Advance time by one frame
        //             pmove_current_clone->cmd.serverTime = pmove_current_clone->oldcmd.serverTime;
        //             pmove_current_clone->cmd.serverTime += 1000 / com_maxfps->current.integer;

        //             // Desired world yaw angle
        //             float desiredYaw = 85.0f;

        //             // Get delta angles from current snapshot
        //             auto &delta_angles = cgArray[localClientNum].activeSnapshots[0].ps.delta_angles;

        //             // Convert delta_angles[1] to degrees
        //             float deltaYaw = SHORT2ANGLE(delta_angles[1]);

        //             // Convert desired world yaw into input-relative cmd.angle
        //             pmove_current_clone->cmd.angles[0] = 0;
        //             pmove_current_clone->cmd.angles[1] = ANGLE2SHORT(desiredYaw - deltaYaw);
        //             pmove_current_clone->cmd.angles[2] = 0;

        //             // Predict!
        //             PmoveSingle(pmove_current_clone);

        //             return pmove_current_clone;
        //         }

        inline clientActive_t &CG_GetLocalClientGlobals(int localClientNum)
        {
            return (*clients)[localClientNum];
        }

        void AutoBHOP_Holding_Jump()
        {
            auto localClientNum = 0;

            pmove_t *pmove_current = &cg_pmove[localClientNum];

            clientActive_t *localClientGlobals = &(*clients)[localClientNum];

            auto angles = localClientGlobals->viewangles;
            DbgPrint("localClientGlobals->viewangles: %d %d %d\n", angles[0], angles[1], angles[2]);

            usercmd_s *cmd = &localClientGlobals->cmds[localClientGlobals->cmdNumber & 0x7F];

            auto pmove_predicted = predict_pmove(localClientNum);

            // 1022 = On ground, 1023 = In air
            auto in_air = pmove_current->ps->groundEntityNum == 1023;
            auto on_ground_predicted = pmove_predicted->ps->groundEntityNum == 1022;

            if (in_air && on_ground_predicted)
            {
                // Clear the jump button for the current frame
                cmd->buttons &= ~1024; // JUMP
            }

            // Clean up the prediction
            delete_pmove(pmove_predicted);
        }

        // Semi working
        // void BlocRPG(int localClientNum)
        // {
        //     pmove_t *pmove_current = &cg_pmove[localClientNum];
        //     auto pmove_predicted = predict_pmove(localClientNum);

        //     auto ca = &(*clients)[localClientNum];
        //     auto cg = &(*cgArray)[localClientNum];
        //     auto cmd = &ca->cmds[ca->cmdNumber & 0x7F];
        //     // auto oldcmd = &ca->cmds[(ca->cmdNumber - 1) & 0x7F];

        //     static auto rpg_mp_index = BG_FindWeaponIndexForName("rpg_mp");
        //     auto holding_rpg = pmove_current->ps->weapon == rpg_mp_index;
        //     auto reloading = pmove_current->ps->weaponstate == WEAPON_RELOADING;

        //     bool shot_rpg_next_frame = pmove_predicted->ps->weaponDelay <= 3 && pmove_predicted->ps->weaponDelay != 0;

        //     if (shot_rpg_next_frame && holding_rpg && !reloading)
        //     {
        //         // Local game
        //         ca->viewangles[PITCH] = AngleNormalize360(50 - cg->snap->ps.delta_angles[PITCH]);
        //         ca->viewangles[YAW] = AngleNormalize360(-90 - cg->snap->ps.delta_angles[YAW]);

        //         // cmd
        //         cmd->angles[PITCH] = ANGLE2SHORT(AngleNormalize360(50 - cg->snap->ps.delta_angles[PITCH]));
        //         cmd->angles[YAW] = ANGLE2SHORT(AngleNormalize360(-90 - cg->snap->ps.delta_angles[YAW]));
        //     }

        //     // cmd->angles[PITCH] = ANGLE2SHORT(AngleNormalize360(50 - cg->snap->ps.delta_angles[PITCH]));
        //     // cmd->angles[YAW] = ANGLE2SHORT(AngleNormalize360(-90 - cg->snap->ps.delta_angles[YAW]));

        //     delete_pmove(pmove_predicted);
        // }

        void BlocRPG(int localClientNum)
        {
            pmove_t *pmove_current = &cg_pmove[localClientNum];
            auto pmove_predicted = predict_pmove(localClientNum, 2);

            auto ca = &(*clients)[localClientNum];
            auto cg = &(*cgArray)[localClientNum];
            auto cmd = &ca->cmds[ca->cmdNumber & 0x7F];

            static auto should_reset = false;
            static auto previous_pitch = ca->viewangles[PITCH];
            static auto previous_yaw = ca->viewangles[YAW];
            if (should_reset)
            {
                // Reset the view angles to the previous state
                ca->viewangles[PITCH] = previous_pitch;
                ca->viewangles[YAW] = previous_yaw;
                should_reset = false;
            }

            static auto rpg_mp_index = BG_FindWeaponIndexForName("rpg_mp");
            auto holding_rpg = pmove_current->ps->weapon == rpg_mp_index;
            auto reloading = pmove_current->ps->weaponstate == WEAPON_RELOADING;

            bool shot_rpg_next_frame = pmove_predicted->ps->weaponDelay <= 3 && pmove_predicted->ps->weaponDelay != 0;

            if (shot_rpg_next_frame && holding_rpg && !reloading)
            {
                previous_pitch = ca->viewangles[PITCH];
                previous_yaw = ca->viewangles[YAW];
                should_reset = true;

                const auto pitch_offset = 70.0f; // Adjust this value as needed
                const auto yaw_offset = -90.0f; // Adjust this value as needed

                // Local game
                ca->viewangles[PITCH] = AngleNormalize360(pitch_offset - cg->snap->ps.delta_angles[PITCH]);
                ca->viewangles[YAW] = AngleNormalize360(yaw_offset - cg->snap->ps.delta_angles[YAW]);

                // cmd
                cmd->angles[PITCH] = ANGLE2SHORT(AngleNormalize360(pitch_offset - cg->snap->ps.delta_angles[PITCH]));
                cmd->angles[YAW] = ANGLE2SHORT(AngleNormalize360(yaw_offset - cg->snap->ps.delta_angles[YAW]));
            }

            delete_pmove(pmove_predicted);
        }

        Detour CL_CreateNewCommands_Detour;

        void CL_CreateNewCommands_Hook(int localClientNum)
        {
            CL_CreateNewCommands_Detour.GetOriginal<decltype(CL_CreateNewCommands)>()(localClientNum);
            if (clientUIActives[localClientNum].connectionState == CA_ACTIVE)
            {
                BlocRPG(localClientNum);
            }
        }

        Detour CL_FinishMove_Detour;

        void CL_FinishMove_Hook(int localClientNum, usercmd_s *cmd)
        {
            CL_FinishMove_Detour.GetOriginal<decltype(CL_FinishMove)>()(localClientNum, cmd);

            // if (clientUIActives[localClientNum].connectionState == CA_ACTIVE)
            // {
            //     BlocRPG(localClientNum);
            // }
        }

        cj_tas::cj_tas()
        {
            CL_CreateNewCommands_Detour = Detour(CL_CreateNewCommands, CL_CreateNewCommands_Hook);
            CL_CreateNewCommands_Detour.Install();

            // CL_FinishMove_Detour = Detour(CL_FinishMove, CL_FinishMove_Hook);
            // CL_FinishMove_Detour.Install();

            cj_tas_jump_at_edge = Dvar_RegisterBool("cj_tas_jump_at_edge", true, 0, "");
            // cj_tas_jump_on_shoot_rpg = Dvar_RegisterBool("cj_tas_jump_on_shoot_rpg", true, 0, "Jump on next frame if we shoot RPG");
        }

        cj_tas::~cj_tas()
        {
        }
    }
}
