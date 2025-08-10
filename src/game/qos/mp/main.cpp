#include "components/g_scr_main.h"
#include "components/scr_parser.h"
#include "main.h"
#include "common.h"

namespace qos
{
namespace mp
{

QOS_MP_Plugin::QOS_MP_Plugin()
{
    DbgPrint("QOS SP: Registering modules\n");
    RegisterModule(new g_scr_main());
    RegisterModule(new scr_parser());
}

bool QOS_MP_Plugin::ShouldLoad()
{
    return strncmp((char *)0x8200236C, "multiplayer", 11) == 0;
}

} // namespace mp
} // namespace qos
