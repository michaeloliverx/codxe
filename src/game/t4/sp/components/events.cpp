#include "pch.h"
#include "events.h"

#include "splitscreen.h"

namespace t4
{
namespace sp
{
namespace
{
typedef void (*EventHandler)();

const EventHandler dvarInitHandlers[] = {
    SplitScreen::OnDvarInit,
};
} // namespace

void Events::Com_InitDvars_Hook()
{
    // Let the game initialize its dvars before registering plugin dvars.
    Com_InitDvars_Detour.GetOriginal<decltype(Com_InitDvars)>()();

    for (size_t i = 0; i < ARRAYSIZE(dvarInitHandlers); ++i)
    {
        dvarInitHandlers[i]();
    }
}

Detour Events::Com_InitDvars_Detour;

Events::Events()
{
    Com_InitDvars_Detour = Detour(Com_InitDvars, Com_InitDvars_Hook);
    Com_InitDvars_Detour.Install();
}

Events::~Events()
{
    Com_InitDvars_Detour.Remove();
}
} // namespace sp
} // namespace t4
