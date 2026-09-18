#include "pch.h"
#include "components/cg.h"
#include "components/cj_tas.h"
#include "components/clipmap.h"
#include "components/command.h"
#include "components/cmds.h"
#include "components/console.h"
#include "components/events.h"
#include "components/fastfiles.h"
#include "components/gsc.h"
#include "components/gsc_fields.h"
#include "components/gsc_functions.h"
#include "components/image_loader.h"
#include "components/mpsp.h"
#include "components/mods.h"
#include "components/patches.h"
#include "components/pm.h"
#include "components/scr_parser.h"
#include "components/stats.h"
#include "components/sv_bots.h"
#include "components/ui_feeder.h"
#include "components/ui_script.h"
#include "main.h"

namespace iw3
{
namespace mp
{
IW3_MP_Plugin::IW3_MP_Plugin()
{
    // default loc_warnings off to prevent console spam
    *(volatile uint8_t *)0x821FB069 = 0xE1;

    // Special modules need to be registered first
    RegisterModule(new Patches());
    RegisterModule(new Events());
    RegisterModule(new command());
    RegisterModule(new UIScript());
    RegisterModule(new UIFeeder());

    RegisterModule(new cg());
    RegisterModule(new cj_tas());
    RegisterModule(new clipmap());
    RegisterModule(new cmds());
    RegisterModule(new console());
    RegisterModule(new ModList());
    RegisterModule(new FastFiles());
    RegisterModule(new GSC());
    RegisterModule(new GSCFields());
    RegisterModule(new GSCFunctions());
    RegisterModule(new image_loader());
    RegisterModule(new pm());
    RegisterModule(new mpsp());
    RegisterModule(new scr_parser());
    RegisterModule(new Stats());
    RegisterModule(new sv_bots());
}

IW3_MP_Plugin::~IW3_MP_Plugin()
{
}

} // namespace mp
} // namespace iw3
