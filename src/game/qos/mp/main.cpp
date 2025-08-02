#include "components/g_scr_main.h"
#include "components/scr_parser.h"
#include "main.h"
#include "common.h"

namespace qos
{
    namespace mp
    {
        std::vector<Module *> components;

        void RegisterComponent(Module *module)
        {
            DbgPrint("QOS SP: Component registered: %s\n", module->get_name());
            components.push_back(module);
        }

        void init()
        {
            DbgPrint("QOS SP: Registering modules\n");
            RegisterComponent(new g_scr_main());
            RegisterComponent(new scr_parser());
        }

        void shutdown()
        {
            // Clean up in reverse order
            for (auto it = components.rbegin(); it != components.rend(); ++it)
                delete *it;

            components.clear();
        }
    }
}
