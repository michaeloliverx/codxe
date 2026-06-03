#include "pch.h"
#include "events.h"

namespace iw3
{
namespace mp
{
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

std::vector<std::function<void()>> Events::cg_init_callbacks;

void Events::CG_Init_Hook(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum)
{
    // Call original function first
    CG_Init_Detour.GetOriginal<CG_Init_t>()(localClientNum, serverMessageNum, serverCommandSequence, clientNum);

    for (auto it = cg_init_callbacks.begin(); it != cg_init_callbacks.end(); ++it)
    {
        (*it)();
    }
}

void Events::OnCG_Init(const std::function<void()> &callback)
{
    cg_init_callbacks.emplace_back(callback);
}

Detour Events::CG_Init_Detour;

std::vector<std::function<void()>> Events::vmshutdown_callbacks;

void Events::Scr_ShutdownSystem_Hook(unsigned __int8 sys)
{
    for (auto it = vmshutdown_callbacks.begin(); it != vmshutdown_callbacks.end(); ++it)
    {
        (*it)();
    }

    // Call original function after callbacks
    Scr_ShutdownSystem_Detour.GetOriginal<Scr_ShutdownSystem_t>()(sys);
}

void Events::OnVMShutdown(const std::function<void()> &callback)
{
    vmshutdown_callbacks.emplace_back(callback);
}

Detour Events::Scr_ShutdownSystem_Detour;

std::vector<std::function<void()>> Events::dvarinit_callbacks;

void Events::Com_InitDvars_Hook()
{
    // Let the game initialize its dvar subsystem first, then register plugin dvars before engine init continues.
    Com_InitDvars_Detour.GetOriginal<Com_InitDvars_t>()();

    for (auto it = dvarinit_callbacks.begin(); it != dvarinit_callbacks.end(); ++it)
    {
        (*it)();
    }

    dvarinit_callbacks.clear();
}

void Events::OnDvarInit(const std::function<void()> &callback)
{
    dvarinit_callbacks.emplace_back(callback);
}

Detour Events::Com_InitDvars_Detour;

std::vector<std::function<void()>> Events::cmdinit_callbacks;

void Events::Cmd_Init_Hook()
{
    // Call original function first so the command subsystem is ready.
    Cmd_Init_Detour.GetOriginal<Cmd_Init_t>()();

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

Events::Events()
{
    CG_DrawActive_Detour = Detour(CG_DrawActive, CG_DrawActive_Hook);
    CG_DrawActive_Detour.Install();

    CG_Init_Detour = Detour(CG_Init, CG_Init_Hook);
    CG_Init_Detour.Install();

    Scr_ShutdownSystem_Detour = Detour(Scr_ShutdownSystem, Scr_ShutdownSystem_Hook);
    Scr_ShutdownSystem_Detour.Install();

    Com_InitDvars_Detour = Detour(Com_InitDvars, Com_InitDvars_Hook);
    Com_InitDvars_Detour.Install();

    Cmd_Init_Detour = Detour(Cmd_Init, Cmd_Init_Hook);
    Cmd_Init_Detour.Install();
}

Events::~Events()
{
    CG_DrawActive_Detour.Remove();
    CG_Init_Detour.Remove();
    Scr_ShutdownSystem_Detour.Remove();
    Com_InitDvars_Detour.Remove();
    Cmd_Init_Detour.Remove();

    cg_drawactive_callbacks.clear();
    cg_init_callbacks.clear();
    vmshutdown_callbacks.clear();
    dvarinit_callbacks.clear();
    cmdinit_callbacks.clear();
}

} // namespace mp
} // namespace iw3
