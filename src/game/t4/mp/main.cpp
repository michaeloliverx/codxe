#include "common.h"
#include "main.h"
#include "modules.h"

namespace t4
{
    namespace mp
    {
        void init()
        {
            DbgPrint("T4 MP: Initializing modules\n");

            register_modules();

            // Log modules initialization
            DbgPrint("T4 MP: Initialized %d modules\n", g_modules.size());

            for (size_t i = 0; i < g_modules.size(); ++i)
            {
                DbgPrint("  - Module: %s\n", g_modules[i]->get_name());
            }
        }

        void shutdown()
        {
            DbgPrint("T4 MP: Shutting down modules\n");
            cleanup_modules();
        }
    }
}
