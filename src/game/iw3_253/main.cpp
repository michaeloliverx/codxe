#include <xtl.h>

#include "cg_consolecmds.h"
#include "scr_parser.h"
#include "structs.h"
#include "..\..\detour.h"

extern "C" void DbgPrint(const char *format, ...);

namespace iw3_253
{
    typedef dvar_s *(*Dvar_FindMalleableVar_t)(const char *dvarName);
    Dvar_FindMalleableVar_t Dvar_FindMalleableVar = reinterpret_cast<Dvar_FindMalleableVar_t>(0x821E0780);

    typedef dvar_s *(*Dvar_RegisterBool_t)(const char *dvarName, bool value, unsigned __int16 flags, const char *description);
    Dvar_RegisterBool_t Dvar_RegisterBool = reinterpret_cast<Dvar_RegisterBool_t>(0x821E1718);

    typedef dvar_s *(*Dvar_RegisterInt_t)(const char *dvarName, int value, int min, int max, unsigned __int16 flags, const char *description);
    Dvar_RegisterInt_t Dvar_RegisterInt = reinterpret_cast<Dvar_RegisterInt_t>(0x821E1788);

    typedef void (*PM_FoliageSounds_t)(pmove_t *pm);
    typedef void (*Pmove_t)(pmove_t *pm);
    typedef void (*PmoveSingle_t)(pmove_t *pm);

    PM_FoliageSounds_t PM_FoliageSounds = reinterpret_cast<PM_FoliageSounds_t>(0x820B27F8);
    Pmove_t Pmove = reinterpret_cast<Pmove_t>(0x820B4F48);
    PmoveSingle_t PmoveSingle = reinterpret_cast<PmoveSingle_t>(0x820B4440);

    typedef void (*CG_DrawActive_t)(int localClientNum);
    CG_DrawActive_t CG_DrawActive = reinterpret_cast<CG_DrawActive_t>(0x820CA770);

    typedef int (*R_RegisterFont_t)(const char *name);
    R_RegisterFont_t R_RegisterFont = reinterpret_cast<R_RegisterFont_t>(0x8216EC00);

    typedef void (*R_AddCmdDrawText_t)(const char *text, int maxChars, Font_s *font, double x, double y, double xScale, double yScale, double rotation, const float *color, int style);
    R_AddCmdDrawText_t R_AddCmdDrawText = (R_AddCmdDrawText_t)0x8228F348;

    typedef bool (*Cmd_ExecFromFastFile_t)(int localClientNum, int controllerIndex, const char *filename);
    Cmd_ExecFromFastFile_t Cmd_ExecFromFastFile = reinterpret_cast<Cmd_ExecFromFastFile_t>(0x8223AF40);

    typedef void (*Cbuf_AddText_t)(int localClientNum, const char *text);
    Cbuf_AddText_t Cbuf_AddText = reinterpret_cast<Cbuf_AddText_t>(0x82183458);

    typedef void (*Cbuf_ExecuteBuffer_t)(int localClientNum, const char *buffer);
    Cbuf_ExecuteBuffer_t Cbuf_ExecuteBuffer = reinterpret_cast<Cbuf_ExecuteBuffer_t>(0x821838D0);

    struct scr_entref_t
    {
        unsigned __int16 entnum;
        unsigned __int16 classnum;
    };

    typedef void (*BuiltinMethod)(scr_entref_t);
    typedef BuiltinMethod (*Scr_GetMethod_t)(const char **pName, int *type);
    Scr_GetMethod_t Scr_GetMethod = reinterpret_cast<Scr_GetMethod_t>(0x82167998);

    typedef char *(*Scr_GetString_t)(unsigned int index);
    Scr_GetString_t Scr_GetString = reinterpret_cast<Scr_GetString_t>(0x821AC760);

    typedef void (*PM_UpdateSprint_t)(pmove_t *pm, const pml_t *pml);
    PM_UpdateSprint_t PM_UpdateSprint = reinterpret_cast<PM_UpdateSprint_t>(0x820ADD50);

    ScreenPlacement &scrPlaceFullUnsafe = *reinterpret_cast<ScreenPlacement *>(0x831ACDB8);

    cgMedia_t &cgMedia = *reinterpret_cast<cgMedia_t *>(0x82848250);

    Detour PM_UpdateSprint_Detour;

    void PM_UpdateSprint_Hook(pmove_t *pm, const pml_t *pml)
    {
        DbgPrint("PM_UpdateSprint_Hook\n");
        PM_UpdateSprint_Detour.GetOriginal<decltype(PM_UpdateSprint)>()(pm, pml);

        // Works but prevents bouncing and mantling
        // pm->ps->pm_flags &= ~0x1401Cu;

        // pm->ps->sprintState.lastSprintEnd = 0;
    }

    Detour Pmove_Detour;

    // https://github.com/kejjjjj/iw3sptool/blob/17b669233a1ad086deed867469dc9530b84c20e6/iw3sptool/bg/bg_pmove.cpp#L11
    void Pmove_Hook(pmove_t *pm)
    {
        static dvar_s *pm_fixed_fps_enable = Dvar_FindMalleableVar("pm_fixed_fps_enable");
        static dvar_s *pm_fixed_fps = Dvar_FindMalleableVar("pm_fixed_fps");

        if (!pm_fixed_fps_enable->current.enabled)
            return Pmove_Detour.GetOriginal<decltype(Pmove)>()(pm);

        int msec = 0;
        int cur_msec = 1000 / pm_fixed_fps->current.integer;

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

    bool g_pm_foliage_ran_this_frame = false;

    void Sys_SnapVector_RoundToNearestEven(float *v)
    {
        // Use __frnd for round-to-nearest-even behavior
        v[0] = (float)__frnd((double)v[0]);
        v[1] = (float)__frnd((double)v[1]);
        v[2] = (float)__frnd((double)v[2]);
    }

    Detour PmoveSingle_Detour;

    void PmoveSingle_Hook(pmove_t *pm)
    {
        DbgPrint("PmoveSingle_Hook\n");
        g_pm_foliage_ran_this_frame = false;
        PmoveSingle_Detour.GetOriginal<decltype(PmoveSingle)>()(pm);
        if (g_pm_foliage_ran_this_frame)
        {
            DbgPrint("Sys_SnapVector_RoundToNearestEven\n");
            Sys_SnapVector_RoundToNearestEven(pm->ps->velocity);
        }

        g_pm_foliage_ran_this_frame = false;
    }

    Detour PM_FoliageSounds_Detour;

    void PM_FoliageSounds_Hook(pmove_t *pm)
    {
        DbgPrint("PM_FoliageSounds_Hook\n");
        PM_FoliageSounds_Detour.GetOriginal<decltype(PM_FoliageSounds)>()(pm);
        g_pm_foliage_ran_this_frame = true;
    }

    void DrawFixedFPS()
    {
        static dvar_s *pm_fixed_fps = Dvar_FindMalleableVar("pm_fixed_fps");

        char buff[16];
        sprintf_s(buff, "%d", pm_fixed_fps->current.integer);

        float color[4] = {1, 1, 1, 1};
        float x = 620 * scrPlaceFullUnsafe.scaleVirtualToFull[0];
        float y = 15 * scrPlaceFullUnsafe.scaleVirtualToFull[1];
        R_AddCmdDrawText(buff, 16, cgMedia.bigDevFont, x, y, 1.0, 1.0, 0.0, color, 0);
    }

    Detour CG_DrawActive_Detour;

    void CG_DrawActive_Hook(int localClientNum)
    {
        static dvar_s *pm_fixed_fps_enable = Dvar_FindMalleableVar("pm_fixed_fps_enable");

        if (pm_fixed_fps_enable->current.enabled)
        {
            DrawFixedFPS();
        }

        CG_DrawActive_Detour.GetOriginal<decltype(CG_DrawActive)>()(localClientNum);
    }

    void GScr_Cbuf_ExecuteBuffer(scr_entref_t entref)
    {
        auto text = Scr_GetString(0);
        Cbuf_ExecuteBuffer(0, text);
    }

    Detour Scr_GetMethod_Detour;

    BuiltinMethod Scr_GetMethod_Hook(const char **pName, int *type)
    {
        if (std::strcmp(*pName, "cbuf_executebuffer") == 0)
            return reinterpret_cast<BuiltinMethod>(&GScr_Cbuf_ExecuteBuffer);

        return Scr_GetMethod_Detour.GetOriginal<decltype(Scr_GetMethod)>()(pName, type);
    }

    void init()
    {
        Pmove_Detour = Detour(Pmove, Pmove_Hook);
        Pmove_Detour.Install();

        Dvar_RegisterBool("pm_fixed_fps_enable", false, 0, "Enable fixed FPS mode");
        Dvar_RegisterInt("pm_fixed_fps", 250, 0, 1000, 0, "Fixed FPS value");

        init_cg_consolecmds();
        init_scr_parser();

        PmoveSingle_Detour = Detour(PmoveSingle, PmoveSingle_Hook);
        PmoveSingle_Detour.Install();

        PM_FoliageSounds_Detour = Detour(PM_FoliageSounds, PM_FoliageSounds_Hook);
        PM_FoliageSounds_Detour.Install();

        // Nop the function that calls Sys_SnapVector
        uintptr_t start = 0x820B4EB4;
        uintptr_t end = 0x820B4F34;

        for (uintptr_t addr = start; addr <= end; addr += 4)
        {
            *(uint32_t *)addr = 0x60000000; // PowerPC NOP
        }

        CG_DrawActive_Detour = Detour(CG_DrawActive, CG_DrawActive_Hook);
        CG_DrawActive_Detour.Install();

        Scr_GetMethod_Detour = Detour(Scr_GetMethod, Scr_GetMethod_Hook);
        Scr_GetMethod_Detour.Install();

        PM_UpdateSprint_Detour = Detour(PM_UpdateSprint, PM_UpdateSprint_Hook);
        PM_UpdateSprint_Detour.Install();

        // Remove read-only protection from DVARs

        //         .text:821E04F4                 bl        _Com_Printf__YAXHPBDZZ # Com_Printf(int,char const *,...)
        // .text:821E04F8                 lwz       r3, 0x4C0+var_40(r1) # cookie
        // .text:821E04FC                 bl        __security_check_cookie
        // .text:821E0500                 addi      r1, r1, 0x4C0
        // .text:821E0504                 b         __restgprlr_27

        // DVARS
        // Patch read-only check to always succeed
        *(unsigned int *)0x821E04E0 = 0x48000028; // b loc_821E0508
        // Patch write protection check to always succeed
        *(unsigned int *)0x821E0510 = 0x48000028; // b loc_821E0538
    }
}
