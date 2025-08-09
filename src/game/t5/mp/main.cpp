#include "components/scr_parser.h"
#include "main.h"
#include "common.h"

namespace t5
{
namespace mp
{
std::vector<Module *> components;

void RegisterComponent(Module *module)
{
    DbgPrint("T4 SP: Component registered: %s\n", module->get_name());
    components.push_back(module);
}

void init()
{
    DbgPrint("t5 SP: Registering modules\n");
    RegisterComponent(new scr_parser());
}

void shutdown()
{
    // Clean up in reverse order
    for (auto it = components.rbegin(); it != components.rend(); ++it)
        delete *it;

    components.clear();
}
} // namespace mp
} // namespace t5
