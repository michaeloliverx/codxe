#include "components/clipmap.h"
#include "components/g_client_fields.h"
#include "components/g_client_script_cmd.h"
#include "components/g_scr_main.h"
#include "components/g_scr_mover.h"
#include "components/scr_parser.h"
#include "components/ui.h"
#include "main.h"
#include "common.h"

namespace t4
{
namespace sp
{

T4_SP_Plugin::T4_SP_Plugin()
{
    DbgPrint("T4 SP: Plugin loaded\n");

    RegisterModule(new clipmap());
    RegisterModule(new g_scr_main()); // Needs to be registered before g_client_script_cmd
    RegisterModule(new g_client_fields());
    RegisterModule(new g_client_script_cmd());
    RegisterModule(new g_scr_mover());
    RegisterModule(new scr_parser());
    RegisterModule(new ui());
}

bool T4_SP_Plugin::ShouldLoad()
{
    return (strncmp((char *)0x82035A94, "startSingleplayer", 17) == 0);
}

} // namespace sp
} // namespace t4
