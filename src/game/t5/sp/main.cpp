#include "pch.h"
#include "components/scr_parser.h"
#include "main.h"

namespace t5
{
namespace sp
{

T5_SP_Plugin::T5_SP_Plugin()
{
    DbgPrint("T5 SP: Registering modules\n");
    RegisterModule(new Config());
    RegisterModule(new scr_parser());
}

} // namespace sp
} // namespace t5
