#include "pch.h"
#include "events.h"

namespace t4
{
namespace mp
{

std::vector<std::function<void()>> Events::dvarinit_callbacks;

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

Events::Events()
{
    Com_InitDvars_Detour = Detour(Com_InitDvars, Com_InitDvars_Hook);
    Com_InitDvars_Detour.Install();
}

Events::~Events()
{
    Com_InitDvars_Detour.Remove();
    dvarinit_callbacks.clear();
}

} // namespace mp
} // namespace t4
