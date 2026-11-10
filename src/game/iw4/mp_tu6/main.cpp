#include "pch.h"
#include "components/cg.h"
#include "components/clipmap.h"
#include "components/cmds.h"
#include "components/console.h"
#include "components/debug_draw.h"
#include "components/events.h"
#include "components/fastfiles.h"
#include "components/gsc_fields.h"
#include "components/gsc.h"
#include "components/mr.h"
#include "components/patches.h"
#include "components/pm.h"
#include "components/scr_parser.h"
#include "components/stats.h"
#include "components/sv_bots.h"
#include "main.h"
#include "components/ui_feeder.h"
#include "components/ui_script.h"
#include "components/mods.h"
#include "components/arenas.h"
#include "components/usermaps.h"
#include "components/usermap_preview.h"

namespace iw4
{
namespace mp_tu6
{

IW4_MP_TU6_Plugin::IW4_MP_TU6_Plugin()
{
    RegisterModule(new Events()); // Must be registered first to ensure hooks are in place
    RegisterModule(new fastfiles());
    RegisterModule(new UIFeeder());
    RegisterModule(new UIScript());
    RegisterModule(new ModList());
    RegisterModule(new Arenas());
    RegisterModule(new Usermaps());
    RegisterModule(new UsermapPreview());

    RegisterModule(new cg());
    RegisterModule(new DebugDraw());
    RegisterModule(new clipmap());
    RegisterModule(new cmds());
    RegisterModule(new console());
    RegisterModule(new GSCFields());
    RegisterModule(new GSC());
    RegisterModule(new MovementRecorder());
    RegisterModule(new patches());
    RegisterModule(new pm());
    RegisterModule(new scr_parser());
    RegisterModule(new stats());
    RegisterModule(new SVBots());
}

} // namespace mp_tu6
} // namespace iw4
