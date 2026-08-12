#include "pch.h"
#include "events.h"

#include "brush_collision.h"
#include "console.h"
#include "stats.h"
#include "sv_bots.h"

namespace t4
{
namespace mp
{
namespace
{
typedef void (*EventHandler)();

const EventHandler dvarInitHandlers[] = {
    BrushCollision::OnDvarInit,
};

const EventHandler cmdInitHandlers[] = {
    stats::OnCmdInit,
};

const EventHandler vmShutdownHandlers[] = {
    SVBots::OnVMShutdown,
};

const EventHandler uiRefreshHandlers[] = {
    console::OnUIRefresh,
};
} // namespace

void Events::Com_InitDvars_Hook()
{
    for (size_t i = 0; i < ARRAYSIZE(dvarInitHandlers); ++i)
    {
        dvarInitHandlers[i]();
    }

    Com_InitDvars_Detour.GetOriginal<decltype(Com_InitDvars)>()();
}

Detour Events::Com_InitDvars_Detour;

void Events::Cmd_Init_Hook()
{
    Cmd_Init_Detour.GetOriginal<Cmd_Init_t>()();

    for (size_t i = 0; i < ARRAYSIZE(cmdInitHandlers); ++i)
    {
        cmdInitHandlers[i]();
    }
}

Detour Events::Cmd_Init_Detour;

void *Events::Scr_ShutdownSystem_Hook(scriptInstance_t inst, int sys, int bComplete)
{
    for (size_t i = 0; i < ARRAYSIZE(vmShutdownHandlers); ++i)
    {
        vmShutdownHandlers[i]();
    }

    return Scr_ShutdownSystem_Detour.GetOriginal<Scr_ShutdownSystem_t>()(inst, sys, bComplete);
}

Detour Events::Scr_ShutdownSystem_Detour;

int Events::UI_Refresh_Hook(int localClientNum)
{
    const int result = UI_Refresh_Detour.GetOriginal<UI_Refresh_t>()(localClientNum);

    for (size_t i = 0; i < ARRAYSIZE(uiRefreshHandlers); ++i)
    {
        uiRefreshHandlers[i]();
    }

    return result;
}

Detour Events::UI_Refresh_Detour;

Events::Events()
{
    Com_InitDvars_Detour = Detour(Com_InitDvars, Com_InitDvars_Hook);
    Com_InitDvars_Detour.Install();

    Cmd_Init_Detour = Detour(Cmd_Init, Cmd_Init_Hook);
    Cmd_Init_Detour.Install();

    Scr_ShutdownSystem_Detour = Detour(Scr_ShutdownSystem, Scr_ShutdownSystem_Hook);
    Scr_ShutdownSystem_Detour.Install();

    UI_Refresh_Detour = Detour(UI_Refresh, UI_Refresh_Hook);
    UI_Refresh_Detour.Install();
}

Events::~Events()
{
    Com_InitDvars_Detour.Remove();
    Cmd_Init_Detour.Remove();
    Scr_ShutdownSystem_Detour.Remove();
    UI_Refresh_Detour.Remove();
}

} // namespace mp
} // namespace t4
