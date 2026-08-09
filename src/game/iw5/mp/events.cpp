#include "pch.h"
#include "events.h"

namespace iw5
{
namespace mp
{

std::vector<std::function<void()>> Events::com_initdvars_callbacks;
std::vector<std::function<void(bool)>> Events::vm_shutdown_callbacks;
Detour Events::Com_InitDvars_Detour;
Detour Events::G_ShutdownGame_Detour;

void Events::Com_InitDvars_Hook()
{
    Com_InitDvars_Detour.GetOriginal<Com_InitDvars_t>()();

    for (auto it = com_initdvars_callbacks.begin(); it != com_initdvars_callbacks.end(); ++it)
    {
        (*it)();
    }

    com_initdvars_callbacks.clear();
}

void Events::G_ShutdownGame_Hook(int freeScripts)
{
    G_ShutdownGame_Detour.GetOriginal<G_ShutdownGame_t>()(freeScripts);

    for (auto it = vm_shutdown_callbacks.begin(); it != vm_shutdown_callbacks.end(); ++it)
    {
        (*it)(freeScripts != 0);
    }
}

void Events::OnDvarInit(const std::function<void()> &callback)
{
    com_initdvars_callbacks.emplace_back(callback);
}

void Events::OnVMShutdown(const std::function<void(bool)> &callback)
{
    vm_shutdown_callbacks.emplace_back(callback);
}

Events::Events()
{
    Com_InitDvars_Detour = Detour(Com_InitDvars, Com_InitDvars_Hook);
    Com_InitDvars_Detour.Install();

    G_ShutdownGame_Detour = Detour(G_ShutdownGame, G_ShutdownGame_Hook);
    G_ShutdownGame_Detour.Install();
}

Events::~Events()
{
    Com_InitDvars_Detour.Remove();
    G_ShutdownGame_Detour.Remove();
    com_initdvars_callbacks.clear();
    vm_shutdown_callbacks.clear();
}

} // namespace mp
} // namespace iw5
