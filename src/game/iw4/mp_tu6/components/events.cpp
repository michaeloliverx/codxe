#include "pch.h"
#include "events.h"

std::vector<std::function<void()>> Events::com_initdvars_callbacks;
Detour Events::Com_InitDvars_Detour;

void Events::Com_InitDvars_Hook()
{
    Com_InitDvars_Detour.GetOriginal<decltype(iw4::mp_tu6::Com_InitDvars)>()();

    for (auto it = com_initdvars_callbacks.begin(); it != com_initdvars_callbacks.end(); ++it)
    {
        (*it)();
    }

    com_initdvars_callbacks.clear();
}

void Events::OnDvarInit(const std::function<void()> &callback)
{
    com_initdvars_callbacks.emplace_back(callback);
}

std::vector<std::function<void()>> Events::cg_drawactive_callbacks;

void Events::CG_DrawActive_Hook(int localClientNum)
{
    // Call original function first
    CG_DrawActive_Detour.GetOriginal<decltype(&CG_DrawActive_Hook)>()(localClientNum);

    for (auto it = cg_drawactive_callbacks.begin(); it != cg_drawactive_callbacks.end(); ++it)
    {
        (*it)();
    }
}

void Events::OnCG_DrawActive(const std::function<void()> &callback)
{
    cg_drawactive_callbacks.emplace_back(callback);
}

Detour Events::CG_DrawActive_Detour;

std::vector<std::function<void()>> Events::cmdinit_callbacks;

void Events::Cmd_Init_Hook()
{
    // Call original function first so the command subsystem is ready.
    Cmd_Init_Detour.GetOriginal<decltype(iw4::mp_tu6::Cmd_Init)>()();

    for (auto it = cmdinit_callbacks.begin(); it != cmdinit_callbacks.end(); ++it)
    {
        (*it)();
    }

    cmdinit_callbacks.clear();
}

void Events::OnCmdInit(const std::function<void()> &callback)
{
    cmdinit_callbacks.emplace_back(callback);
}

Detour Events::Cmd_Init_Detour;

std::vector<std::function<void()>> Events::vmshutdown_callbacks;

void Events::Scr_ShutdownSystem_Hook(unsigned __int8 sys)
{
    for (auto it = vmshutdown_callbacks.begin(); it != vmshutdown_callbacks.end(); ++it)
    {
        (*it)();
    }

    Scr_ShutdownSystem_Detour.GetOriginal<iw4::mp_tu6::Scr_ShutdownSystem_t>()(sys);
}

void Events::OnVMShutdown(const std::function<void()> &callback)
{
    vmshutdown_callbacks.emplace_back(callback);
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

    com_initdvars_callbacks.clear();
    cg_drawactive_callbacks.clear();
    cmdinit_callbacks.clear();
    vmshutdown_callbacks.clear();
}
