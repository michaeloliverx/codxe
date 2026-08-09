#include "pch.h"
#include "components/clipmap.h"
#include "components/gsc.h"
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
    RegisterModule(new GSC());
    RegisterModule(new GSCFields());
    RegisterModule(new scr_parser());
}

} // namespace mp
} // namespace qos
