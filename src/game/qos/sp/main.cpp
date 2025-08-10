#include "components/scr_parser.h"
#include "main.h"
#include "common.h"

namespace qos
{
namespace sp
{
QOS_SP_Plugin::QOS_SP_Plugin()
{
    DbgPrint("QOS SP: Registering modules\n");
    RegisterModule(new scr_parser());
}

bool QOS_SP_Plugin::ShouldLoad()
{
    return strncmp((char *)0x82047EC4, "startSingleplayer", 17) == 0;
}

} // namespace sp
} // namespace qos
