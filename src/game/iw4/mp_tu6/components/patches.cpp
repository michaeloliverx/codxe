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

void AllowMixedLanguageFastfiles()
{
    // Every fastfile header carries a language mask (4 bytes after the build time: English 0x1, Russian 0x40,
    // Japanese 0x400). The first zone loaded sets the game's mask (0x825CE4E4); any later zone that shares no
    // bit with it ends in Com_Error "Language mismatch on download content." - which is what the English title
    // update's patch_mp.ff does to another language's disc data, as an endless relaunch.
    // Take the "languages match" branch always, so a localized copy runs on the supported (English) executable
    // and title update without touching its files.
    // .text:821AFE24                 and       r11, r6, r5
    // .text:821AFE28                 cmplwi    cr6, r11, 0
    // .text:821AFE2C                 bne       cr6, loc_821AFE48
    if (*reinterpret_cast<volatile uint32_t *>(0x821AFE2C) == 0x409A001C)
        ppc::Branch(0x821AFE2C, 0x821AFE48);
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
    AllowMixedLanguageFastfiles();
}

patches::~patches()
{
#ifndef NDEBUG
    CL_ConsolePrint_Detour.Remove();
#endif
}
} // namespace mp_tu6
} // namespace iw4
