#include "cj_tas.h"
#include "common.h"
#include <algorithm>

#define ANGLE2SHORT(x) ((int)((x) * 65536 / 360) & 65535)
#define SHORT2ANGLE(x) ((x) * (360.0 / 65536))

namespace iw3
{
    namespace mp
    {
        struct RecordedCmd
        {
            int serverTime;
            int buttons;
            int angles[2];         // PITCH, YAW
            float delta_angles[2]; // PITCH, YAW
            unsigned __int8 weapon;
            unsigned __int8 offHandIndex;
            char forwardmove;
            char rightmove;
        };
        size_t play_frame = 0;
        bool is_recording = false;
        bool is_playing = false;
        int playback_start_time = 0;
        int recording_start_time = 0;
        std::vector<RecordedCmd> current_recording;

        static cmd_function_s Cmd_Startrecord_VAR;
        static cmd_function_s Cmd_Stoprecord_VAR;
        static cmd_function_s Cmd_Togglerecord_VAR;
        static cmd_function_s Cmd_Startplayback_VAR;
        static cmd_function_s Cmd_Stopplayback_VAR;

        dvar_s *cj_tas_rpg_fire_slowdown_trick = nullptr;

        dvar_s *cj_tas_playback_force_rpg = nullptr;

        unsigned int rpg_mp_index = 0;

        void cj_tas::On_CG_Init()
        {
            // Weapon indexes change every game
            rpg_mp_index = BG_FindWeaponIndexForName("rpg_mp");
        }

        void Cmd_Startrecord_f()
        {
            if (is_recording)
            {
                CG_GameMessage(0, "^3Already recording");
                return;
            }

            is_recording = true;
            current_recording.clear();
            CG_GameMessage(0, "Recording ^2started");
        }

        void Cmd_Stoprecord_f()
        {
            if (!is_recording)
            {
                CG_GameMessage(0, "^1Not currently recording.");
                return;
            }

            is_recording = false;
            CG_GameMessage(0, "Recording ^1stopped");
        }

        void Cmd_Togglerecord_f()
        {
            if (is_recording)
            {
                Cmd_Stoprecord_f();
            }
            else
            {
                Cmd_Startrecord_f();
            }
        }

        void Cmd_Startplayback_f()
        {
            if (is_recording)
            {
                CG_GameMessage(0, "^1Stop recording before starting playback.\n");
                return;
            }

            if (is_playing)
            {
                CG_GameMessage(0, "^3Already playing.\n");
                return;
            }

            if (current_recording.empty())
            {
                CG_GameMessage(0, "^1No recording available to play.\n");
                return;
            }

            play_frame = 0;
            is_playing = true;
            playback_start_time = 0; // Will be set on first UpdateCommand
            recording_start_time = current_recording[0].serverTime;
            CG_GameMessage(0, "Playback ^2started\n");
        }

        void Cmd_Stopplayback_f()
        {
            if (!is_playing)
            {
                CG_GameMessage(0, "^1Not currently playing.\n");
                return;
            }

            play_frame = 0;
            is_playing = false;
            CG_GameMessage(0, "Playback ^1stopped\n");
        }

        bool IsPlayback()
        {
            return is_playing;
        }

        void CaptureCommand(usercmd_s *const cmd)
        {
            const auto cg = &(*cgArray)[0];
            const auto ps = &cg->predictedPlayerState;

            RecordedCmd recorded_cmd;
            recorded_cmd.serverTime = cmd->serverTime;
            recorded_cmd.buttons = cmd->buttons;
            recorded_cmd.angles[PITCH] = cmd->angles[PITCH];
            recorded_cmd.angles[YAW] = cmd->angles[YAW];
            recorded_cmd.delta_angles[PITCH] = ps->delta_angles[PITCH];
            recorded_cmd.delta_angles[YAW] = ps->delta_angles[YAW];
            recorded_cmd.weapon = cmd->weapon;
            recorded_cmd.offHandIndex = cmd->offHandIndex;
            recorded_cmd.forwardmove = cmd->forwardmove;
            recorded_cmd.rightmove = cmd->rightmove;

            current_recording.push_back(recorded_cmd);
        }

        void UpdateCommand(usercmd_s *const cmd)
        {
            if (current_recording.empty())
                return;

            if (play_frame >= current_recording.size())
            {
                Cmd_Stopplayback_f();
                return;
            }

            const auto cg = &(*cgArray)[0];
            auto ca = &(*clients)[0];
            const auto &data = current_recording[play_frame];

            // Initialize playback start time on first frame
            if (playback_start_time == 0)
            {
                playback_start_time = cmd->serverTime;
            }

            // Calculate the relative time offset from the start of the recording
            int recording_time_offset = data.serverTime - recording_start_time;

            // Apply this offset to the current playback time
            cmd->serverTime = playback_start_time + recording_time_offset;

            const auto ps_delta_pitch = ANGLE2SHORT(cg->predictedPlayerState.delta_angles[PITCH]);
            const auto ps_delta_yaw = ANGLE2SHORT(cg->predictedPlayerState.delta_angles[YAW]);

            const auto expectated_pitch = data.angles[PITCH] + ANGLE2SHORT(data.delta_angles[PITCH]);
            const auto expectated_yaw = data.angles[YAW] + ANGLE2SHORT(data.delta_angles[YAW]);

            const auto net_pitch = expectated_pitch - ps_delta_pitch;
            const auto net_yaw = expectated_yaw - ps_delta_yaw;

            const int movementThreshold = 75;

            if (std::abs(cmd->forwardmove) >= movementThreshold ||
                std::abs(cmd->rightmove) >= movementThreshold)
            {
                Cmd_Stopplayback_f();
                // Set client viewangles to match the recorded angles
                ca->viewangles[PITCH] = static_cast<float>(SHORT2ANGLE(net_pitch));
                ca->viewangles[YAW] = static_cast<float>(SHORT2ANGLE(net_yaw));
                return;
            }

            cmd->buttons |= data.buttons;
            // Set the command angles to the recorded angles
            cmd->angles[PITCH] = net_pitch;
            cmd->angles[YAW] = net_yaw;

            // Set client viewangles to match the recorded angles
            ca->viewangles[PITCH] = static_cast<float>(SHORT2ANGLE(net_pitch));
            ca->viewangles[YAW] = static_cast<float>(SHORT2ANGLE(net_yaw));

            cmd->weapon = data.weapon;

            if (cj_tas_playback_force_rpg->current.enabled)
            {
                cmd->weapon = static_cast<unsigned char>(rpg_mp_index); // Force the weapon to be RPG
            }

            cmd->offHandIndex = data.offHandIndex;
            cmd->forwardmove = data.forwardmove;
            cmd->rightmove = data.rightmove;

            play_frame++;
        }

        dvar_s *cj_tas_bhop_auto = nullptr;

        dvar_s *cj_tas_jump_at_edge = nullptr;

        dvar_s *cj_tas_jump_on_rpg_fire = nullptr;

        dvar_s *cj_tas_crouch_on_jump = nullptr;

        dvar_s *cj_tas_rpg_lookdown = nullptr;
        dvar_s *cj_tas_rpg_lookdown_yaw = nullptr;
        dvar_s *cj_tas_rpg_lookdown_pitch = nullptr;

        bool cj_tas::TAS_Enabled()
        {
            const bool tas_enabled = (IsPlayback() ||
                                      cj_tas_bhop_auto->current.enabled ||
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
                ca->viewangles[PITCH] = previous_pitch;
                ca->viewangles[YAW] = previous_yaw;
                should_reset = false;
            }

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

                auto ca = &(*clients)[localClientNum];
                auto cmd = &ca->cmds[ca->cmdNumber & 0x7F];

                if (is_recording)
                {
                    CaptureCommand(cmd);
                }
                if (is_playing)
                {
                    UpdateCommand(cmd);
                }

                TAS_Cycle(localClientNum);
            }
        }

        cj_tas::cj_tas()
        {
            CL_CreateNewCommands_Detour = Detour(CL_CreateNewCommands, CL_CreateNewCommands_Hook);
            CL_CreateNewCommands_Detour.Install();

            Cmd_AddCommandInternal("startrecord", Cmd_Startrecord_f, &Cmd_Startrecord_VAR);
            Cmd_AddCommandInternal("stoprecord", Cmd_Stoprecord_f, &Cmd_Stoprecord_VAR);
            Cmd_AddCommandInternal("togglerecord", Cmd_Togglerecord_f, &Cmd_Togglerecord_VAR);
            Cmd_AddCommandInternal("startplayback", Cmd_Startplayback_f, &Cmd_Startplayback_VAR);
            Cmd_AddCommandInternal("stopplayback", Cmd_Stopplayback_f, &Cmd_Stopplayback_VAR);

            cj_tas_playback_force_rpg = Dvar_RegisterBool("cj_tas_playback_force_rpg", false, 0, "Force playback to equip RPG");

            cj_tas_bhop_auto = Dvar_RegisterBool("cj_tas_bhop_auto", false, 0, "Enable automatic bunny hopping");

            cj_tas_jump_at_edge = Dvar_RegisterBool("cj_tas_jump_at_edge", false, 0, "Enable jump at edge");

            cj_tas_jump_on_rpg_fire = Dvar_RegisterBool("cj_tas_jump_on_rpg_fire", false, 0, "Jump exactly when firing the RPG");

            cj_tas_crouch_on_jump = Dvar_RegisterBool("cj_tas_crouch_on_jump", false, 0, "Enable crouch on jump");

            cj_tas_rpg_lookdown = Dvar_RegisterBool("cj_tas_rpg_lookdown", false, 0, "Enable RPG lookdown");
            cj_tas_rpg_lookdown_yaw = Dvar_RegisterInt("cj_tas_rpg_lookdown_yaw", 0, -180, 180, 0, "RPG lookdown yaw angle");
            cj_tas_rpg_lookdown_pitch = Dvar_RegisterInt("cj_tas_rpg_lookdown_pitch", 70, -70, 70, 0, "RPG lookdown pitch angle");
        }

        cj_tas::~cj_tas()
        {
            CL_CreateNewCommands_Detour.Remove();
        }
    }
}
