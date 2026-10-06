#include "pch.h"
#include "debug.h"

namespace t4
{
namespace mp
{
Detour Debug::ConsolePrintDetour;

void Debug::CL_ConsolePrint_Hook(int channel, const char *text, int flags)
{
    DbgPrint("[codxe][T4][CL_ConsolePrint] %s", text);
    ConsolePrintDetour.GetOriginal<CL_ConsolePrint_t>()(channel, text, flags);
}

Debug::Debug()
{
    ConsolePrintDetour = Detour(CL_ConsolePrint, CL_ConsolePrint_Hook);
    ConsolePrintDetour.Install();
}

Debug::~Debug()
{
    ConsolePrintDetour.Remove();
}
} // namespace mp
} // namespace t4
