#include "components/cg.h"
#include "components/clipmap.h"
#include "components/g_client_fields.h"
#include "components/g_scr_main.h"
#include "components/patches.h"
#include "components/scr_parser.h"
#include "main.h"
#include "common.h"

namespace iw4
{
namespace mp
{

IW4_MP_Plugin::IW4_MP_Plugin()
{

    DbgPrint("IW4 MP: Registering modules\n");
    RegisterModule(new cg());
    RegisterModule(new clipmap());
    RegisterModule(new g_client_fields());
    RegisterModule(new g_scr_main());
    RegisterModule(new patches());
    RegisterModule(new scr_parser());
}

bool IW4_MP_Plugin::ShouldLoad()
{
    DbgPrint("IW4 MP: Checking if plugin should load\n");
    return (strncmp((char *)0x82001F44, "multiplayer", 11) == 0);
}

} // namespace mp
} // namespace iw4
