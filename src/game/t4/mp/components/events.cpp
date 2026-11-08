#include "pch.h"
#include "events.h"

#include "brush_collision.h"
#include "console.h"
#include "collision.h"
#include "debug_draw.h"
#include "gsc_functions.h"
#include "mods.h"
#include "usermaps.h"
#include "stats.h"
#include "sv_bots.h"

namespace t4
{
namespace mp
{
namespace
{
typedef void (*EventHandler)();
typedef void (*CGDrawHandler)(int localClientNum);

const CGDrawHandler cgDrawActiveHandlers[] = {
    DebugDraw::OnCGDrawActive,
    Collision::OnCGDrawActive,
};

const EventHandler dvarInitHandlers[] = {
    BrushCollision::OnDvarInit,
    Collision::OnDvarInit,
    ModList::OnDvarInit,
    Usermaps::OnDvarInit,
};

const EventHandler cmdInitHandlers[] = {
    stats::OnCmdInit,
};

const EventHandler vmShutdownHandlers[] = {
    GSCFunctions::OnVMShutdown,
    SVBots::OnVMShutdown,
};

const EventHandler uiRefreshHandlers[] = {
    console::OnUIRefresh,
};
} // namespace

void Events::CG_DrawActive_Hook(int localClientNum)
{
    BrushCollision::OnCGDrawActive(localClientNum);
    CG_DrawActive_Detour.GetOriginal<decltype(CG_DrawActive)>()(localClientNum);
    for (size_t i = 0; i < ARRAYSIZE(cgDrawActiveHandlers); ++i)
        cgDrawActiveHandlers[i](localClientNum);
}
Detour Events::CG_DrawActive_Detour;

void Events::CG_Init_Hook(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum)
{
    Collision::OnCGInit();
    CG_Init_Detour.GetOriginal<decltype(CG_Init)>()(localClientNum, serverMessageNum, serverCommandSequence, clientNum);
}
Detour Events::CG_Init_Detour;

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

void *Events::Scr_ShutdownSystem_Hook(scriptInstance_t inst, scrSystem_t sys, int bComplete)
{
    // These handlers own server script state and run only on complete shutdowns.
    if (inst == SCRIPTINSTANCE_SERVER && bComplete)
    {
        for (size_t i = 0; i < ARRAYSIZE(vmShutdownHandlers); ++i)
        {
            vmShutdownHandlers[i]();
        }
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
    CG_DrawActive_Detour = Detour(CG_DrawActive, CG_DrawActive_Hook);
    CG_DrawActive_Detour.Install();
    CG_Init_Detour = Detour(CG_Init, CG_Init_Hook);
    CG_Init_Detour.Install();

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
    CG_DrawActive_Detour.Remove();
    CG_Init_Detour.Remove();
    Com_InitDvars_Detour.Remove();
    Cmd_Init_Detour.Remove();
    Scr_ShutdownSystem_Detour.Remove();
    UI_Refresh_Detour.Remove();
}

} // namespace mp
} // namespace t4
