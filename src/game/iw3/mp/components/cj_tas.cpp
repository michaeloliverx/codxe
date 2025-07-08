#include "cj_tas.h"
#include "common.h"

#define ANGLE2SHORT(x) ((int)((x) * 65536 / 360) & 65535)

namespace iw3
{
    namespace mp
    {
        dvar_s *cj_tas_bhop_auto = nullptr;

        dvar_s *cj_tas_jump_at_edge = nullptr;

        dvar_s *cj_tas_jump_on_rpg_fire = nullptr;

        dvar_s *cj_tas_crouch_on_jump = nullptr;

        dvar_s *cj_tas_rpg_lookdown = nullptr;
        dvar_s *cj_tas_rpg_lookdown_yaw = nullptr;
        dvar_s *cj_tas_rpg_lookdown_pitch = nullptr;

        bool cj_tas::TAS_Enabled()
        {
            const bool tas_enabled = (cj_tas_bhop_auto->current.enabled ||
                                      cj_tas_jump_at_edge->current.enabled ||
                                      cj_tas_jump_on_rpg_fire->current.enabled ||
                                      cj_tas_crouch_on_jump->current.enabled ||
                                      cj_tas_rpg_lookdown->current.enabled);
            return tas_enabled;
        }

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

        pmove_t *predict_pmove(int localClientNum, int framesToAdvance = 1)
        {
            // On console this is 85!
            // vsync is 60 FPS, so we predict at 60 FPS
            // TODO: investigate if this is correct
            // static auto com_maxfps = Dvar_FindMalleableVar("com_maxfps");

            pmove_t *pmove_current = &cg_pmove[localClientNum];
            auto pmove_clone = clone_pmove(pmove_current);

            auto ca = &(*clients)[localClientNum];

            int frameTime = 1000 / 60; // assuming 60 FPS
            for (int i = 0; i < framesToAdvance; ++i)
            {
                pmove_clone->oldcmd = pmove_clone->cmd;
                pmove_clone->cmd.serverTime += frameTime;

                pmove_clone->cmd.angles[PITCH] = ANGLE2SHORT(ca->viewangles[PITCH]);
                pmove_clone->cmd.angles[YAW] = ANGLE2SHORT(ca->viewangles[YAW]);

                PmoveSingle(pmove_clone);
            }

            return pmove_clone;
        }

        void TAS_Cycle(int localClientNum)
        {
            if (!cj_tas::TAS_Enabled())
                return;

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

            if (cj_tas_jump_on_rpg_fire->current.enabled && shot_rpg_next_frame && holding_rpg && !reloading)
            {
                cmd->buttons |= 1024; // JUMP
            }

            if (cj_tas_rpg_lookdown->current.enabled && shot_rpg_next_frame && holding_rpg && !reloading)
            {
                previous_pitch = ca->viewangles[PITCH];
                previous_yaw = ca->viewangles[YAW];
                should_reset = true;

                const auto pitch_offset = cj_tas_rpg_lookdown_pitch->current.integer;
                const auto yaw_offset = cj_tas_rpg_lookdown_yaw->current.integer;

                // Local game
                ca->viewangles[PITCH] = AngleNormalize360(pitch_offset - cg->snap->ps.delta_angles[PITCH]);
                ca->viewangles[YAW] = AngleNormalize360(yaw_offset - cg->snap->ps.delta_angles[YAW]);

                // cmd
                cmd->angles[PITCH] = ANGLE2SHORT(AngleNormalize360(pitch_offset - cg->snap->ps.delta_angles[PITCH]));
                cmd->angles[YAW] = ANGLE2SHORT(AngleNormalize360(yaw_offset - cg->snap->ps.delta_angles[YAW]));

                // // Invert movement direction ?
                // cmd->forwardmove = -cmd->forwardmove;
                // cmd->rightmove = -cmd->rightmove;
            }

            bool is_on_ground = pmove_current->ps->groundEntityNum == 1022;              // 1022 = On ground
            bool is_in_air = pmove_current->ps->groundEntityNum == 1023;                 // 1023 = In air
            bool is_on_ground_next_frame = pmove_predicted->ps->groundEntityNum == 1022; // 1022 = On ground
            bool is_in_air_next_frame = pmove_predicted->ps->groundEntityNum == 1023;    // 1023 = In air
            bool will_leave_ground_this_frame = is_on_ground && is_in_air_next_frame;

            if (cj_tas_jump_at_edge->current.enabled && will_leave_ground_this_frame)
            {
                cmd->buttons |= 1024; // JUMP
            }

            if (cj_tas_bhop_auto->current.enabled && is_in_air && is_on_ground_next_frame)
            {
                // Clear the jump button if we are in the air and will land next frame
                cmd->buttons &= ~1024; // Clear JUMP button
            }

            if (cj_tas_crouch_on_jump->current.enabled && cmd->buttons & 1024)
            {
                CL_SetStance(localClientNum, CL_STANCE_CROUCH);
            }

            delete_pmove(pmove_predicted);
        }

        Detour CL_CreateNewCommands_Detour;

        void CL_CreateNewCommands_Hook(int localClientNum)
        {
            CL_CreateNewCommands_Detour.GetOriginal<decltype(CL_CreateNewCommands)>()(localClientNum);
            if (clientUIActives[localClientNum].connectionState == CA_ACTIVE)
            {
                TAS_Cycle(localClientNum);
            }
        }

        cj_tas::cj_tas()
        {
            CL_CreateNewCommands_Detour = Detour(CL_CreateNewCommands, CL_CreateNewCommands_Hook);
            CL_CreateNewCommands_Detour.Install();

            cj_tas_bhop_auto = Dvar_RegisterBool("cj_tas_bhop_auto", false, 0, "Enable automatic bunny hopping");

            cj_tas_jump_at_edge = Dvar_RegisterBool("cj_tas_jump_at_edge", false, 0, "Enable jump at edge");

            cj_tas_jump_on_rpg_fire = Dvar_RegisterBool("cj_tas_jump_on_rpg_fire", false, 0, "Jump exactly when firing the RPG");

            cj_tas_crouch_on_jump = Dvar_RegisterBool("cj_tas_crouch_on_jump", false, 0, "Enable crouch on jump");

            cj_tas_rpg_lookdown = Dvar_RegisterBool("cj_tas_rpg_lookdown", false, 0, "Enable RPG lookdown");
            cj_tas_rpg_lookdown_yaw = Dvar_RegisterInt("cj_tas_rpg_lookdown_yaw", 0, -180, 180, 0, "RPG lookdown yaw angle in degrees");
            cj_tas_rpg_lookdown_pitch = Dvar_RegisterInt("cj_tas_rpg_lookdown_pitch", 70, -70, 70, 0, "RPG lookdown pitch angle in degrees");
        }

        cj_tas::~cj_tas()
        {
            CL_CreateNewCommands_Detour.Remove();
        }
    }
}
