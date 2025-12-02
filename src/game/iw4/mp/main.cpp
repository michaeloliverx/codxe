#include "pch.h"
#include "components/cg.h"
#include "components/clipmap.h"
#include "components/console.h"
#include "components/events.h"
#include "components/g_client_fields.h"
#include "components/g_scr_main.h"
#include "components/mr.h"
#include "components/patches.h"
#include "components/scr_parser.h"
#include "main.h"

namespace iw4
{
namespace mp
{

IW4_MP_Plugin::IW4_MP_Plugin()
{

    DbgPrint("IW4 MP: Registering modules\n");
    RegisterModule(new Events()); // Must be registered first to ensure hooks are in place
    RegisterModule(new cg());
    RegisterModule(new clipmap());
    RegisterModule(new Console());
    RegisterModule(new g_client_fields());
    RegisterModule(new g_scr_main());
    RegisterModule(new MovementRecorder());
    RegisterModule(new patches());
    RegisterModule(new scr_parser());
}

} // namespace mp
} // namespace iw4
