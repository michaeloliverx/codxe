#include "pch.h"
#include "events.h"

namespace t4
{
namespace mp
{

std::vector<std::function<void()>> Events::dvarinit_callbacks;
std::vector<std::function<void()>> Events::vmshutdown_callbacks;

void Events::Com_InitDvars_Hook()
{
    for (auto it = dvarinit_callbacks.begin(); it != dvarinit_callbacks.end(); ++it)
    {
        (*it)();
    }

    dvarinit_callbacks.clear();

    Com_InitDvars_Detour.GetOriginal<decltype(Com_InitDvars)>()();
}

void Events::OnDvarInit(const std::function<void()> &callback)
{
    dvarinit_callbacks.emplace_back(callback);
}

Detour Events::Com_InitDvars_Detour;

void *Events::Scr_ShutdownSystem_Hook(scriptInstance_t inst, int sys, int bComplete)
{
    for (auto it = vmshutdown_callbacks.begin(); it != vmshutdown_callbacks.end(); ++it)
    {
        (*it)();
    }

    return Scr_ShutdownSystem_Detour.GetOriginal<Scr_ShutdownSystem_t>()(inst, sys, bComplete);
}

void Events::OnVMShutdown(const std::function<void()> &callback)
{
    vmshutdown_callbacks.emplace_back(callback);
}

Detour Events::Scr_ShutdownSystem_Detour;

Events::Events()
{
    Com_InitDvars_Detour = Detour(Com_InitDvars, Com_InitDvars_Hook);
    Com_InitDvars_Detour.Install();

    Scr_ShutdownSystem_Detour = Detour(Scr_ShutdownSystem, Scr_ShutdownSystem_Hook);
    Scr_ShutdownSystem_Detour.Install();
}

Events::~Events()
{
    Com_InitDvars_Detour.Remove();
    Scr_ShutdownSystem_Detour.Remove();

    dvarinit_callbacks.clear();
    vmshutdown_callbacks.clear();
}

} // namespace mp
} // namespace t4
