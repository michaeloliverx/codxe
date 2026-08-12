#include "pch.h"
#include "events.h"

#include "main.h"
#include "pm.h"

namespace iw5
{
namespace mp
{
namespace
{
typedef void (*EventHandler)();
typedef void (*VMShutdownHandler)(bool freeScripts);

const EventHandler dvarInitHandlers[] = {
    PlayerMovement::OnDvarInit,
};

const VMShutdownHandler vmShutdownHandlers[] = {
    IW5_MP_Plugin::OnVMShutdown,
};
} // namespace

Detour Events::Com_InitDvars_Detour;
Detour Events::G_ShutdownGame_Detour;

void Events::Com_InitDvars_Hook()
{
    Com_InitDvars_Detour.GetOriginal<Com_InitDvars_t>()();

    for (size_t i = 0; i < ARRAYSIZE(dvarInitHandlers); ++i)
    {
        dvarInitHandlers[i]();
    }
}

void Events::G_ShutdownGame_Hook(int freeScripts)
{
    G_ShutdownGame_Detour.GetOriginal<G_ShutdownGame_t>()(freeScripts);

    const bool shouldFreeScripts = freeScripts != 0;
    for (size_t i = 0; i < ARRAYSIZE(vmShutdownHandlers); ++i)
    {
        vmShutdownHandlers[i](shouldFreeScripts);
    }
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
}

} // namespace mp
} // namespace iw5
