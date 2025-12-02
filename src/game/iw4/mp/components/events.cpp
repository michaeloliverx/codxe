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

Events::Events()
{
    CG_DrawActive_Detour = Detour(iw4::mp::CG_DrawActive, CG_DrawActive_Hook);
    CG_DrawActive_Detour.Install();
}

Events::~Events()
{
    CG_DrawActive_Detour.Remove();
}
