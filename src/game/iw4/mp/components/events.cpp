#include "pch.h"
#include "events.h"

#include "pm.h"

namespace iw4
{
namespace mp
{
namespace
{
typedef void (*EventHandler)();

const EventHandler dvarInitHandlers[] = {
    pm::OnDvarInit,
};
} // namespace

Detour Events::Com_InitDvars_Detour;

void Events::Com_InitDvars_Hook()
{
    Com_InitDvars_Detour.GetOriginal<Com_InitDvars_t>()();

    for (size_t i = 0; i < ARRAYSIZE(dvarInitHandlers); ++i)
    {
        dvarInitHandlers[i]();
    }
}

Events::Events()
{
    Com_InitDvars_Detour = Detour(Com_InitDvars, Events::Com_InitDvars_Hook);
    Com_InitDvars_Detour.Install();
}

Events::~Events()
{
    Com_InitDvars_Detour.Remove();
}
} // namespace mp
} // namespace iw4
