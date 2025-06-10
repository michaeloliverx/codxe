#pragma once
#include <vector>

namespace t4
{
    namespace mp
    {
        class Module
        {
        public:
            Module() {};
            virtual ~Module() {};
            virtual const char *get_name() { return "unknown"; }
        };

        // Global vector of modules
        extern std::vector<Module *> g_modules;

        // Simple registration function
        void register_modules();

        // Function to clean up modules
        void cleanup_modules();

    }
}
