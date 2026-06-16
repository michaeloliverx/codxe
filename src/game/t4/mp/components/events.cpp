#include "pch.h"
#include "events.h"

namespace t4
{
namespace mp
{

std::vector<std::function<void()>> Events::dvarinit_callbacks;
std::vector<std::function<void()>> Events::cmdinit_callbacks;
std::vector<std::function<void()>> Events::vmshutdown_callbacks;
std::vector<std::function<void()>> Events::ui_refresh_callbacks;

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

void Events::Cmd_Init_Hook()
{
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

int Events::UI_Refresh_Hook(int localClientNum)
{
    const int result = UI_Refresh_Detour.GetOriginal<UI_Refresh_t>()(localClientNum);

    for (auto it = ui_refresh_callbacks.begin(); it != ui_refresh_callbacks.end(); ++it)
    {
        (*it)();
    }

    return result;
}

void Events::OnUIRefresh(const std::function<void()> &callback)
{
    ui_refresh_callbacks.emplace_back(callback);
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

    dvarinit_callbacks.clear();
    cmdinit_callbacks.clear();
    vmshutdown_callbacks.clear();
    ui_refresh_callbacks.clear();
}

} // namespace mp
} // namespace t4
