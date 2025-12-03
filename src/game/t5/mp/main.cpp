#include "pch.h"
#include "components/scr_parser.h"
#include "main.h"

namespace t5
{
namespace mp
{

T5_MP_Plugin::T5_MP_Plugin()
{
    DbgPrint("T5 MP: Registering modules\n");
    RegisterModule(new Config());
    RegisterModule(new scr_parser());
}

} // namespace mp
} // namespace t5
