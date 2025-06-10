#include "modules.h"
#include "components/branding.h"
#include "components/gsc_loader.h"
#include "components/image_loader.h"
#include "components/test_module.h"

namespace t4
{
    namespace mp
    {

        std::vector<Module *> g_modules;

        void register_modules()
        {
            g_modules.push_back(new Branding());
            g_modules.push_back(new GSCLoader());
            g_modules.push_back(new ImageLoader());
            g_modules.push_back(new TestModule());
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
