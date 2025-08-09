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
std::vector<Module *> components;

void RegisterComponent(Module *module)
{
    DbgPrint("T4 SP: Component registered: %s\n", module->get_name());
    components.push_back(module);
}

void init()
{
    DbgPrint("T4 SP: Registering modules\n");
    RegisterComponent(new clipmap());
    RegisterComponent(new g_scr_main()); // Needs to be registered before g_client_script_cmd
    RegisterComponent(new g_client_fields());
    RegisterComponent(new g_client_script_cmd());
    RegisterComponent(new g_scr_mover());
    RegisterComponent(new scr_parser());
    RegisterComponent(new ui());
}

void shutdown()
{
    // Clean up in reverse order
    for (auto it = components.rbegin(); it != components.rend(); ++it)
        delete *it;

    components.clear();
}
} // namespace sp
} // namespace t4
