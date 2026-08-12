#include "pch.h"
#include "events.h"

#include "command.h"

namespace iw3
{
namespace sp
{
namespace
{
typedef void (*EventHandler)();

const EventHandler cmdInitHandlers[] = {
    command::OnCmdInit,
};
} // namespace

void Events::Cmd_Init_Hook()
{
    // Call original first so the command subsystem is ready for plugin commands.
    Cmd_Init_Detour.GetOriginal<Cmd_Init_t>()();

    for (size_t i = 0; i < ARRAYSIZE(cmdInitHandlers); ++i)
    {
        cmdInitHandlers[i]();
    }
}

Detour Events::Cmd_Init_Detour;

Events::Events()
{
    Cmd_Init_Detour = Detour(Cmd_Init, Cmd_Init_Hook);
    Cmd_Init_Detour.Install();
}

Events::~Events()
{
    Cmd_Init_Detour.Remove();
}
} // namespace sp
} // namespace iw3
