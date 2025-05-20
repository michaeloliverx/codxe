#pragma once

#include <vector>
#include <memory>
#include <algorithm>

#include "module.h"

namespace common
{
    class ModuleRegistry
    {
    private:
        std::vector<std::unique_ptr<Module>> modules;

        // Private constructor for singleton
        ModuleRegistry() {};

    public:
        // Singleton accessor
        static ModuleRegistry &getInstance()
        {
            static ModuleRegistry instance;
            return instance;
        }

        // Destructor automatically cleans up modules in reverse order
        ~ModuleRegistry() {};

        // Register a module (takes ownership)
        template <typename T>
        T *registerModule()
        {
            std::unique_ptr<T> module(new T());
            T *modulePtr = module.get();
            modules.push_back(std::move(module));
            return modulePtr;
        }

        // Get all modules
        const std::vector<std::unique_ptr<Module>> &getModules() const
        {
            return modules;
        }
    };
}
