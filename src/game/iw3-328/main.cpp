#include "structs.h"
#include "cg_consolecmds.h"
#include "scr_parser.h"

namespace iw3_328
{
    struct DvarValueStringBuf
    {
        const char *pad;
        char string[12];
    };

    union DvarValue
    {
        bool enabled;
        int integer;
        unsigned int unsignedInt;
        float value;
        float vector[4];
        const char *string;
        DvarValueStringBuf stringBuf;
        unsigned __int8 color[4];
    };

    union DvarLimits
    {
        struct
        {
            int stringCount;
            const char **strings;
        } enumeration;

        struct
        {
            int min;
            int max;
        } integer;

        struct
        {
            float min;
            float max;
        } value;

        struct
        {
            float min;
            float max;
        } vector;
    };

    struct dvar_s
    {
        const char *name;
        const char *description;
        unsigned __int16 flags;
        unsigned __int8 type;
        bool modified;
        DvarValue current;
        DvarValue latched;
        DvarValue reset;
        DvarLimits domain;
        dvar_s *next;
        dvar_s *hashNext;
    };

    typedef dvar_s *(*Dvar_FindMalleableVar_t)(const char *dvarName);
    // typedef bool (*Dvar_GetBool_t)(const char *dvarName);
    typedef dvar_s *(*Dvar_RegisterBool_t)(const char *dvarName, bool value, unsigned __int16 flags, const char *description);
    // typedef dvar_s *(*Dvar_RegisterColor_t)(const char *dvarName, double r, double g, double b, double a, unsigned __int16 flags, const char *description);
    // typedef dvar_s *(*Dvar_RegisterEnum_t)(const char *dvarName, const char **valueList, unsigned __int16 defaultIndex, unsigned __int16 flags, const char *description);
    typedef dvar_s *(*Dvar_RegisterInt_t)(const char *dvarName, int value, int min, int max, unsigned __int16 flags, const char *description);

    Dvar_FindMalleableVar_t Dvar_FindMalleableVar = reinterpret_cast<Dvar_FindMalleableVar_t>(0x82269CF8);
    // Dvar_GetBool_t Dvar_GetBool = reinterpret_cast<Dvar_GetBool_t>(0x82269EA8);
    Dvar_RegisterBool_t Dvar_RegisterBool = reinterpret_cast<Dvar_RegisterBool_t>(0x8226AF38);
    // Dvar_RegisterColor_t Dvar_RegisterColor = reinterpret_cast<Dvar_RegisterColor_t>(0x821D4D98);
    // Dvar_RegisterEnum_t Dvar_RegisterEnum = reinterpret_cast<Dvar_RegisterEnum_t>(0x821D4F88);
    Dvar_RegisterInt_t Dvar_RegisterInt = reinterpret_cast<Dvar_RegisterInt_t>(0x821D37C8);

    typedef void (*Sys_SnapVector_t)(float *result);
    Sys_SnapVector_t Sys_SnapVector = reinterpret_cast<Sys_SnapVector_t>(0x822A7E18);

    typedef void (*CG_DrawActive_t)(int localClientNum);
    CG_DrawActive_t CG_DrawActive = reinterpret_cast<CG_DrawActive_t>(0x82316020);

    dvar_s *pc_mp_fps_mode = nullptr;

    Detour Sys_SnapVector_Detour;

    void Sys_SnapVector_Hook(float *v)
    {
        // static dvar_s *pm_fps_pc_mode = Dvar_FindMalleableVar("pm_fps_mode");
        if (pc_mp_fps_mode->current.enabled)
        {
            // Use __frnd for round-to-nearest-even behavior
            v[0] = (float)__frnd((double)v[0]);
            v[1] = (float)__frnd((double)v[1]);
            v[2] = (float)__frnd((double)v[2]);
        }
        else
        {
            Sys_SnapVector_Detour.GetOriginal<decltype(Sys_SnapVector)>()(v);
        }
    }

    struct __declspec(align(4)) usercmd_s
    {
        int serverTime;
        int buttons;
        unsigned __int8 weapon;
        unsigned __int8 offHandIndex;
        int angles[3];
        char forwardmove;
        char rightmove;
        float meleeChargeYaw;
        unsigned __int8 meleeChargeDist;
    };

    struct playerState_s
    {
        int commandTime;
    };

    struct __declspec(align(4)) pmove_t
    {
        playerState_s *ps;
        usercmd_s cmd;
        usercmd_s oldcmd;
        int tracemask;
        int numtouch;
        int touchents[32];
        float mins[3];
        float maxs[3];
        float xyspeed;
        int proneChange;
        float maxSprintTimeMultiplier;
        bool mantleStarted;
        float mantleEndPos[3];
        int mantleDuration;
        int viewChangeTime;
        float viewChange;
        unsigned __int8 handler;
    };

    typedef void (*PmoveSingle_t)(pmove_t *pm);
    PmoveSingle_t PmoveSingle = reinterpret_cast<PmoveSingle_t>(0x8210D6B8);

    typedef void (*Pmove_t)(pmove_t *pm);
    Pmove_t Pmove = reinterpret_cast<Pmove_t>(0x8210E0E0);

    Detour Pmove_Detour;

    void Pmove_Hook(pmove_t *pm)
    {
        // usercmd_s *p_cmd;    // r31
        // playerState_s *ps;   // r27
        // int serverTime;      // r29
        // int commandTime;     // r11
        // int v6;              // r11
        // usercmd_s *v7;       // r11
        // usercmd_s *p_oldcmd; // r10
        // int v9;              // ctr
        // int v10;             // r9

        // p_cmd = &pm->cmd;
        // ps = pm->ps;
        // serverTime = pm->cmd.serverTime;
        // commandTime = pm->ps->commandTime;
        // if (serverTime >= commandTime)
        // {
        //     if (serverTime > commandTime + 1000)
        //         ps->commandTime = serverTime - 1000;
        //     pm->numtouch = 0;
        //     while (ps->commandTime != serverTime)
        //     {
        //         v6 = serverTime - ps->commandTime;
        //         if (v6 > 66)
        //             v6 = 66;
        //         p_cmd->serverTime = ps->commandTime + v6;
        //         PmoveSingle(pm);
        //         v7 = p_cmd;
        //         p_oldcmd = &pm->oldcmd;
        //         v9 = 9;
        //         do
        //         {
        //             v10 = v7->serverTime;
        //             v7 = (usercmd_s *)((char *)v7 + 4);
        //             p_oldcmd->serverTime = v10;
        //             p_oldcmd = (usercmd_s *)((char *)p_oldcmd + 4);
        //             --v9;
        //         } while (v9);
        //     }
        // }

        static dvar_s *com_maxfps = Dvar_FindMalleableVar("com_maxfps");

        int msec = 0;
        int cur_msec = 1000 / com_maxfps->current.integer;

        // if (pm_fixed->current.enabled == false)
        //     cur_msec = 66;
        // else
        pm->cmd.serverTime = ((pm->cmd.serverTime + (cur_msec < 2 ? 2 : cur_msec) - 1) / cur_msec) * cur_msec;

        int finalTime = pm->cmd.serverTime;

        if (finalTime < pm->ps->commandTime)
        {
            return; // should not happen
        }

        if (finalTime > pm->ps->commandTime + 1000)
            pm->ps->commandTime = finalTime - 1000;
        pm->numtouch = 0;

        while (pm->ps->commandTime != finalTime)
        {
            msec = finalTime - pm->ps->commandTime;

            if (msec > cur_msec)
                msec = cur_msec;

            pm->cmd.serverTime = msec + pm->ps->commandTime;
            PmoveSingle(pm);
            memcpy(&pm->oldcmd, &pm->cmd, sizeof(pm->oldcmd));
        }
    }

    void init()
    {
        Sys_SnapVector_Detour = Detour(Sys_SnapVector, Sys_SnapVector_Hook);
        Sys_SnapVector_Detour.Install();

        Pmove_Detour = Detour(Pmove, Pmove_Hook);
        Pmove_Detour.Install();

        pc_mp_fps_mode = Dvar_RegisterBool("pc_mp_fps_mode", true, 0, "");
        init_cg_consolecmds();
        init_scr_parser();
    }
}
