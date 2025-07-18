#include "components/clipmap.h"
#include "components/g_client_fields.h"
#include "components/pm.h"
#include "components/scr_parser.h"
#include "main.h"
#include "common.h"

namespace iw4
{
    namespace sp
    {
        std::vector<Module *> components;

        void RegisterComponent(Module *module)
        {
            DbgPrint("IW4 SP: Component registered: %s\n", module->get_name());
            components.push_back(module);
        }

        void init()
        {
            DbgPrint("IW4 SP: Registering modules\n");

            RegisterComponent(new clipmap());
            RegisterComponent(new g_client_fields());
            RegisterComponent(new pm());
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
