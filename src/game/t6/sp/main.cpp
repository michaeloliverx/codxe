#include "pch.h"
#include "components/scr_parser.h"
#include "main.h"

namespace t6
{
namespace sp
{
T6_SP_Plugin::T6_SP_Plugin()
{
    DbgPrint("T6 SP: Registering modules\n");
    RegisterModule(new Config());
    RegisterModule(new scr_parser());
}
} // namespace sp
} // namespace t6
