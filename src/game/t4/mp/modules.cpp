#include "modules.h"
#include "components/branding.h"
#include "components/gsc_client_fields.h"
#include "components/gsc_loader.h"
#include "components/image_loader.h"
#include "components/test_module.h"

namespace t4
{
    namespace mp
    {
        std::vector<Module *> g_modules;

        void RegisterModule(Module *module)
        {
            DbgPrint("T4 MP: Module registered: %s\n", module->get_name());
            g_modules.push_back(module);
        }

        void register_modules()
        {
            DbgPrint("T4 MP: Registering modules\n");
            RegisterModule(new Branding());
            RegisterModule(new GSCClientFields());
            RegisterModule(new GSCLoader());
            // RegisterModule(new ImageLoader());
            RegisterModule(new TestModule());
        }

        void cleanup_modules()
        {
            // Clean up in reverse order
            for (auto it = g_modules.rbegin(); it != g_modules.rend(); ++it)
            {
                delete *it;
            }
            g_modules.clear();
        }
    }
}
