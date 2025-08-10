#include "components/scr_parser.h"
#include "main.h"
#include "common.h"

namespace t5
{
namespace mp
{

T5_MP_Plugin::T5_MP_Plugin()
{
    DbgPrint("T5 MP: Registering modules\n");
    RegisterModule(new scr_parser());
}

bool T5_MP_Plugin::ShouldLoad()
{
    return strncmp((char *)0x82001998, "multiplayer", 11) == 0;
}

} // namespace mp
} // namespace t5
