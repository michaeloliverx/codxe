#include "pch.h"
#include "components/clipmap.h"
#include "components/console.h"
#include "components/fastfiles.h"
#include "components/gsc_fields.h"
#include "components/gsc.h"
#include "components/scr_parser.h"
#include "components/ui.h"
#include "main.h"

namespace t4
{
namespace sp
{

T4_SP_Plugin::T4_SP_Plugin()
{
    // Default loc_warnings off to prevent console spam
    *(volatile uint8_t *)0x8225FA17 = 0x00;

    RegisterModule(new Config(Config::GAME_T4));
    RegisterModule(new FastFiles());
    RegisterModule(new clipmap());
    RegisterModule(new console());
    RegisterModule(new GSC());
    RegisterModule(new GSCFields());
    RegisterModule(new scr_parser());
    RegisterModule(new ui());
}

} // namespace sp
} // namespace t4
