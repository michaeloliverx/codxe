#include "pch.h"
#include "events.h"

namespace iw3
{
namespace sp
{
std::vector<std::function<void()>> Events::cmdinit_callbacks;
std::vector<std::function<void()>> Events::dvarinit_callbacks;

void Events::Cmd_Init_Hook()
{
    // Call original first so the command subsystem is ready for plugin commands.
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

Detour Events::Cmd_Init_Detour;
Detour Events::Com_InitDvars_Detour;

Events::Events()
{
    Cmd_Init_Detour = Detour(Cmd_Init, Cmd_Init_Hook);
    Cmd_Init_Detour.Install();

    Com_InitDvars_Detour = Detour(Com_InitDvars, Com_InitDvars_Hook);
    Com_InitDvars_Detour.Install();
}

Events::~Events()
{
    Cmd_Init_Detour.Remove();
    Com_InitDvars_Detour.Remove();

    cmdinit_callbacks.clear();
    dvarinit_callbacks.clear();
}
} // namespace sp
} // namespace iw3
