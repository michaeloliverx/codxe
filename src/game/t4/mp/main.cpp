#include "common.h"
#include "main.h"
#include "module_registry.h"

namespace t4
{
    namespace mp
    {
        void init()
        {
            // Initialize the game
            xbox::show_notification(L"T4 mp initialized");

            registerModules();

            // Log modules initialization
            const auto &modules = common::ModuleRegistry::getInstance().getModules();
            DbgPrint("T4 MP: Initialized %d modules\n", modules.size());

            for (auto it = modules.begin(); it != modules.end(); ++it)
            {
                DbgPrint("  - Module: %s\n", (*it)->get_name());
            }
        }
    }
}
