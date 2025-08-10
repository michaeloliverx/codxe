#include "components/scr_parser.h"
#include "main.h"
#include "common.h"

namespace t5
{
namespace sp
{

T5_SP_Plugin::T5_SP_Plugin()
{
    DbgPrint("T5 SP: Registering modules\n");
    RegisterModule(new scr_parser());
}

bool T5_SP_Plugin::ShouldLoad()
{
    return strncmp((char *)0x82018364, "startSingleplayer", 17) == 0;
}

} // namespace sp
} // namespace t5
