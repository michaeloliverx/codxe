#include "pch.h"
#include "events.h"

namespace iw3
{
namespace mp
{

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

std::vector<std::function<void()>> Events::cg_init_callbacks;

void Events::CG_Init_Hook(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum)
{
    // Call original function first
    CG_Init_Detour.GetOriginal<CG_Init_t>()(localClientNum, serverMessageNum, serverCommandSequence, clientNum);

    for (auto it = cg_init_callbacks.begin(); it != cg_init_callbacks.end(); ++it)
    {
        (*it)();
    }
}

void Events::OnCG_Init(const std::function<void()> &callback)
{
    cg_init_callbacks.emplace_back(callback);
}

Detour Events::CG_Init_Detour;

Events::Events()
{
    CG_DrawActive_Detour = Detour(CG_DrawActive, CG_DrawActive_Hook);
    CG_DrawActive_Detour.Install();

    CG_Init_Detour = Detour(CG_Init, CG_Init_Hook);
    CG_Init_Detour.Install();
}

Events::~Events()
{
    CG_DrawActive_Detour.Remove();
    CG_Init_Detour.Remove();
}

} // namespace mp
} // namespace iw3
