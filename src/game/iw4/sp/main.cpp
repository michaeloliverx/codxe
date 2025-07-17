#include "components/g_client_fields.h"
#include "components/scr_parser.h"
#include "main.h"
#include "common.h"

namespace iw4
{
    namespace sp
    {
        // PM_StepSlideMove
        void ApplyBounceDepatch()
        {
            *(volatile uint32_t *)0x8232B250 = 0x60000000;
            *(volatile uint32_t *)0x8232B254 = 0x60000000;
        }

        std::vector<Module *> components;

        void RegisterComponent(Module *module)
        {
            DbgPrint("IW4 SP: Component registered: %s\n", module->get_name());
            components.push_back(module);
        }

        void init()
        {
            DbgPrint("IW4 SP: Registering modules\n");

            RegisterComponent(new g_client_fields());
            RegisterComponent(new scr_parser());

            ApplyBounceDepatch();
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
