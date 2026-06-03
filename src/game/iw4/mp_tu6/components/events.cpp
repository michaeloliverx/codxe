#include "pch.h"
#include "events.h"

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

Events::Events()
{
    CG_DrawActive_Detour = Detour(iw4::mp_tu6::CG_DrawActive, CG_DrawActive_Hook);
    CG_DrawActive_Detour.Install();

    Cmd_Init_Detour = Detour(iw4::mp_tu6::Cmd_Init, Cmd_Init_Hook);
    Cmd_Init_Detour.Install();
}

Events::~Events()
{
    CG_DrawActive_Detour.Remove();
    Cmd_Init_Detour.Remove();

    cg_drawactive_callbacks.clear();
    cmdinit_callbacks.clear();
}
