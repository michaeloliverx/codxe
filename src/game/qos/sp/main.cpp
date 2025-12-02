#include "pch.h"
#include "components/scr_parser.h"
#include "main.h"

namespace qos
{
namespace sp
{
QOS_SP_Plugin::QOS_SP_Plugin()
{
    DbgPrint("QOS SP: Registering modules\n");
    RegisterModule(new scr_parser());
}

} // namespace sp
} // namespace qos
