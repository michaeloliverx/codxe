#include "pch.h"
#include "events.h"

namespace iw5
{
namespace mp
{

std::vector<std::function<void(bool)>> Events::vm_shutdown_callbacks;
Detour Events::G_ShutdownGame_Detour;

void Events::G_ShutdownGame_Hook(int freeScripts)
{
    G_ShutdownGame_Detour.GetOriginal<G_ShutdownGame_t>()(freeScripts);

    for (auto it = vm_shutdown_callbacks.begin(); it != vm_shutdown_callbacks.end(); ++it)
    {
        (*it)(freeScripts != 0);
    }
}

void Events::OnVMShutdown(const std::function<void(bool)> &callback)
{
    vm_shutdown_callbacks.emplace_back(callback);
}

Events::Events()
{
    G_ShutdownGame_Detour = Detour(G_ShutdownGame, G_ShutdownGame_Hook);
    G_ShutdownGame_Detour.Install();
}

Events::~Events()
{
    G_ShutdownGame_Detour.Remove();
    vm_shutdown_callbacks.clear();
}

} // namespace mp
} // namespace iw5
