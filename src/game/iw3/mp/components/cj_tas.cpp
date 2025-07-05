#include "cj_tas.h"
#include "common.h"

#define ANGLE2SHORT(x) ((int)((x) * 65536 / 360) & 65535)
#define SHORT2ANGLE(x) ((x) * (360.0 / 65536))

namespace iw3
{
    namespace mp
    {
        dvar_s *cj_tas_jump_at_edge = nullptr;

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
            // bool next_frame_not_on_ground = predicted->ps->groundEntityNum != 1022;
            // if (next_frame_not_on_ground)
            //     cmd->buttons |= 1024; // JUMP

            bool next_frame_in_air = predicted->ps->groundEntityNum == 1023;
            if (next_frame_in_air)
                cmd->buttons |= 1024; // JUMP

            delete_pmove(predicted);
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
            }
        }

        cj_tas::cj_tas()
        {
            CL_CreateNewCommands_Detour = Detour(CL_CreateNewCommands, CL_CreateNewCommands_Hook);
            CL_CreateNewCommands_Detour.Install();

            cj_tas_jump_at_edge = Dvar_RegisterBool("cj_tas_jump_at_edge", true, 0, "");
        }

        cj_tas::~cj_tas()
        {
        }
    }
}
