#include "pch.h"
#include "events.h"

#include "clipmap.h"

namespace qos
{
namespace mp
{
namespace
{
typedef void (*EventHandler)();

const EventHandler cgDrawActiveHandlers[] = {
    clipmap::OnCGDrawActive,
};

const EventHandler cgInitHandlers[] = {
    clipmap::OnCGInit,
};
} // namespace

void Events::CG_DrawActive_Hook(int localClientNum)
{
    // Call original function first
    CG_DrawActive_Detour.GetOriginal<decltype(&CG_DrawActive_Hook)>()(localClientNum);

    for (size_t i = 0; i < ARRAYSIZE(cgDrawActiveHandlers); ++i)
    {
        cgDrawActiveHandlers[i]();
    }
}

Detour Events::CG_DrawActive_Detour;

void Events::CG_Init_Hook(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum)
{
    // Call original function first
    CG_Init_Detour.GetOriginal<CG_Init_t>()(localClientNum, serverMessageNum, serverCommandSequence, clientNum);

    for (size_t i = 0; i < ARRAYSIZE(cgInitHandlers); ++i)
    {
        cgInitHandlers[i]();
    }
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
} // namespace qos
