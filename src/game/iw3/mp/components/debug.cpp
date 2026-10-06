#include "pch.h"
#include "debug.h"

namespace iw3
{
namespace mp
{
Detour Debug::ConsolePrintDetour;

void Debug::CL_ConsolePrint_Hook(int localClientNum, int channel, const char *txt, int duration, int pixelWidth,
                                 int flags)
{
    DbgPrint("[codxe][IW3][CL_ConsolePrint] %s", txt);
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
} // namespace mp
} // namespace iw3
