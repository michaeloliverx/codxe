#include "pch.h"
#include "events.h"

namespace iw4
{
namespace mp
{
std::vector<std::function<void()>> Events::com_initdvars_callbacks;
Detour Events::Com_InitDvars_Detour;

void Events::Com_InitDvars_Hook()
{
    Com_InitDvars_Detour.GetOriginal<Com_InitDvars_t>()();

    for (auto it = com_initdvars_callbacks.begin(); it != com_initdvars_callbacks.end(); ++it)
    {
        (*it)();
    }

    com_initdvars_callbacks.clear();
}

void Events::OnDvarInit(const std::function<void()> &callback)
{
    com_initdvars_callbacks.emplace_back(callback);
}

Events::Events()
{
    Com_InitDvars_Detour = Detour(Com_InitDvars, Events::Com_InitDvars_Hook);
    Com_InitDvars_Detour.Install();
}

Events::~Events()
{
    Com_InitDvars_Detour.Remove();
    com_initdvars_callbacks.clear();
}
} // namespace mp
} // namespace iw4
