#include "pch.h"
#include "debug.h"

namespace t4
{
namespace sp
{
Detour Debug::ConsolePrintDetour;

void Debug::CL_ConsolePrint_Hook(int localClientNum, int channel, const char *txt, int duration, int pixelWidth,
                                 int flags)
{
    DbgPrint("[codxe][CL_ConsolePrint] %s", txt ? txt : "(null)");
    ConsolePrintDetour.GetOriginal<decltype(CL_ConsolePrint)>()(localClientNum, channel, txt, duration, pixelWidth,
                                                                flags);
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
} // namespace sp
} // namespace t4
