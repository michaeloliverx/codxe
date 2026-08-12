#include "pch.h"
#include "events.h"

#include "clipmap.h"
#include "console.h"
#include "gsc.h"
#include "mr.h"
#include "pm.h"
#include "stats.h"
#include "sv_bots.h"

namespace
{
typedef void (*EventHandler)();

const EventHandler dvarInitHandlers[] = {
    iw4::mp_tu6::pm::OnDvarInit,
};

const EventHandler cgDrawActiveHandlers[] = {
    clipmap::OnCGDrawActive,
    iw4::mp_tu6::MovementRecorder::OnCGDrawActive,
};

const EventHandler cmdInitHandlers[] = {
    console::OnCmdInit,
    iw4::mp_tu6::MovementRecorder::OnCmdInit,
    iw4::mp_tu6::stats::OnCmdInit,
};

const EventHandler vmShutdownHandlers[] = {
    iw4::mp_tu6::GSC::OnVMShutdown,
    iw4::mp_tu6::SVBots::OnVMShutdown,
};
} // namespace

Detour Events::Com_InitDvars_Detour;

void Events::Com_InitDvars_Hook()
{
    Com_InitDvars_Detour.GetOriginal<decltype(iw4::mp_tu6::Com_InitDvars)>()();

    for (size_t i = 0; i < ARRAYSIZE(dvarInitHandlers); ++i)
    {
        dvarInitHandlers[i]();
    }
}

void Events::CG_DrawActive_Hook(int localClientNum)
{
    // Call original function first
    CG_DrawActive_Detour.GetOriginal<decltype(&CG_DrawActive_Hook)>()(localClientNum);

    for (size_t i = 0; i < ARRAYSIZE(cgDrawActiveHandlers); ++i)
    {
        cgDrawActiveHandlers[i]();
    }
}

Detour Events::CG_DrawActive_Detour;

void Events::Cmd_Init_Hook()
{
    // Call original function first so the command subsystem is ready.
    Cmd_Init_Detour.GetOriginal<decltype(iw4::mp_tu6::Cmd_Init)>()();

    for (size_t i = 0; i < ARRAYSIZE(cmdInitHandlers); ++i)
    {
        cmdInitHandlers[i]();
    }
}

Detour Events::Cmd_Init_Detour;

void Events::Scr_ShutdownSystem_Hook(unsigned __int8 sys)
{
    for (size_t i = 0; i < ARRAYSIZE(vmShutdownHandlers); ++i)
    {
        vmShutdownHandlers[i]();
    }

    Scr_ShutdownSystem_Detour.GetOriginal<iw4::mp_tu6::Scr_ShutdownSystem_t>()(sys);
}

Detour Events::Scr_ShutdownSystem_Detour;

Events::Events()
{
    Com_InitDvars_Detour = Detour(iw4::mp_tu6::Com_InitDvars, Com_InitDvars_Hook);
    Com_InitDvars_Detour.Install();

    CG_DrawActive_Detour = Detour(iw4::mp_tu6::CG_DrawActive, CG_DrawActive_Hook);
    CG_DrawActive_Detour.Install();

    Cmd_Init_Detour = Detour(iw4::mp_tu6::Cmd_Init, Cmd_Init_Hook);
    Cmd_Init_Detour.Install();

    Scr_ShutdownSystem_Detour = Detour(iw4::mp_tu6::Scr_ShutdownSystem, Scr_ShutdownSystem_Hook);
    Scr_ShutdownSystem_Detour.Install();
}

Events::~Events()
{
    Com_InitDvars_Detour.Remove();
    CG_DrawActive_Detour.Remove();
    Cmd_Init_Detour.Remove();
    Scr_ShutdownSystem_Detour.Remove();
}
