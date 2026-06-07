#include "pch.h"
#include "components/cg.h"
#include "components/clipmap.h"
#include "components/console.h"
#include "components/events.h"
#include "components/g_client_fields.h"
#include "components/g_scr_main.h"
#include "components/mpsp.h"
#include "components/mr.h"
#include "components/patches.h"
#include "components/pm.h"
#include "components/scr_parser.h"
#include "components/sv_bots.h"
#include "main.h"

namespace iw4
{
namespace mp_tu6
{

IW4_MP_TU6_Plugin::IW4_MP_TU6_Plugin()
{
    RegisterModule(new Config());
    RegisterModule(new Events()); // Must be registered first to ensure hooks are in place
    RegisterModule(new cg());
    RegisterModule(new clipmap());
    RegisterModule(new console());
    RegisterModule(new g_client_fields());
    RegisterModule(new g_scr_main());
    RegisterModule(new mpsp());
    RegisterModule(new MovementRecorder());
    RegisterModule(new patches());
    RegisterModule(new pm());
    RegisterModule(new scr_parser());
    RegisterModule(new SVBots());
}

} // namespace mp_tu6
} // namespace iw4
