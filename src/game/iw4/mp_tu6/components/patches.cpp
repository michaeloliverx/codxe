#include "pch.h"
#include "patches.h"

namespace iw4
{
namespace mp_tu6
{

Detour CL_ConsolePrint_Detour;

void CL_ConsolePrint_Hook(int localClientNum, int channel, const char *txt, unsigned int duration,
                          unsigned int pixelWidth, int flags)
{
    DbgPrint("[CL_ConsolePrint] %s", txt);

    CL_ConsolePrint_Detour.GetOriginal<CL_ConsolePrint_t>()(localClientNum, channel, txt, duration, pixelWidth, flags);
}

void DisableFastfileAuth()
{
    // The game requires fastfiles to be signed in MP, but it has the code to load
    // unsigned fastfiles in the executable. Disable the auth check gate.
    ppc::Nop(0x821B0978);
}

void EnableBouncing()
{
    // Re-enable bouncing
    // https://xoxor4d.github.io/research/mw2-bounce/
    // PM_StepSlideMove
    // .text:8210CB70                 cmpwi     cr6, r23, 0
    // .text:8210CB74                 bne       cr6, loc_8210CB24
    ppc::Nop(0x8210CB70);
    ppc::Nop(0x8210CB74);
}

void DisableDvarProtection()
{
    // Read-only
    // .text:8230D680                 b         __restgprlr_27
    ppc::Nop(0x8230D680);

    // Write-protected
    // .text:8230D6A8                 b         __restgprlr_27
    ppc::Nop(0x8230D6A8);

    // Cheat-protected
    // .text:8230D6EC                 b         __restgprlr_27
    ppc::Nop(0x8230D6EC);

    // default migration_dvarErrors off to prevent console spam
    *(volatile uint8_t *)0x822828E7 = 0x0;

    // default loc_warnings off to prevent console spam
    *(volatile uint8_t *)0x822CBDEB = 0x0;
}

patches::patches()
{
#ifndef NDEBUG
    CL_ConsolePrint_Detour = Detour(CL_ConsolePrint, CL_ConsolePrint_Hook);
    CL_ConsolePrint_Detour.Install();
#endif

    DisableFastfileAuth();
    EnableBouncing();
    DisableDvarProtection();
}

patches::~patches()
{
#ifndef NDEBUG
    CL_ConsolePrint_Detour.Remove();
#endif
}
} // namespace mp_tu6
} // namespace iw4
