#include "components/g_client_fields.h"
#include "components/g_scr_mover.h"
#include "components/gsc.h"
#include "components/test_module.h"
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
            RegisterComponent(new g_client_fields());
            RegisterComponent(new g_scr_mover());
            RegisterComponent(new GSC());
            RegisterComponent(new TestModule());
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
