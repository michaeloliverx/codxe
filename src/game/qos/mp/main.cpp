#include "pch.h"
#include "components/clipmap.h"
#include "components/g_scr_main.h"
#include "components/gsc_fields.h"
#include "components/scr_parser.h"
#include "components/events.h"
#include "main.h"

namespace qos
{
namespace mp
{

QOS_MP_Plugin::QOS_MP_Plugin()
{
    DbgPrint("QOS SP: Registering modules\n");
    RegisterModule(new Config());
    RegisterModule(new Events());
    RegisterModule(new clipmap());
    RegisterModule(new g_scr_main());
    RegisterModule(new gsc_fields());
    RegisterModule(new scr_parser());
}

} // namespace mp
} // namespace qos
