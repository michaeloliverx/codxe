#pragma once

#include "common/module_registry.h"
#include "components/asset_dumper.h"

namespace t4
{
    namespace mp
    {
        // Function to register all modules for t4::mp
        inline void registerModules()
        {
            // Get the registry instance
            auto &registry = common::ModuleRegistry::getInstance();

            // Register all modules
            // Each registration returns a pointer to the registered module
            registry.registerModule<AssetDumperModule>();

            // You can perform additional setup with the module pointers if needed
            // Example: assetDumper->dumpAssets();
        }
    }
}
