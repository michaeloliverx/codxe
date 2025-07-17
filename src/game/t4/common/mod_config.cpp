#include "mod_config.h"
#include "common.h"

namespace t4
{
    const char *MOD_DIR = "game:\\_codxe\\mods";
    const char *ACTIVE_MOD_FILE = "game:\\_codxe\\mods\\active_mod.txt";
    const char *DUMP_DIR = "game:\\_codxe\\dump";

    std::string GetModBasePath()
    {
        // Static variables maintain state between function calls
        static std::string cachedModBasePath;
        static bool isCached = false;

        // Return cached value if available
        if (isCached)
            return cachedModBasePath;

        // Read and cache the mod base path
        if (!filesystem::exists(ACTIVE_MOD_FILE))
        {
            cachedModBasePath = "";
            isCached = true;
            return cachedModBasePath;
        }

        std::ifstream activeModFile(ACTIVE_MOD_FILE);
        if (!activeModFile.is_open())
        {
            cachedModBasePath = "";
            isCached = true;
            return cachedModBasePath;
        }

        std::string activeMod;
        std::getline(activeModFile, activeMod);
        activeModFile.close();

        if (activeMod.empty())
        {
            cachedModBasePath = "";
            isCached = true;
            return cachedModBasePath;
        }

        std::string modBasePath = MOD_DIR;
        modBasePath += "\\" + activeMod;

        if (!filesystem::exists(modBasePath))
        {
            DbgPrint("GSCLoader: Active mod directory does not exist: %s\n", modBasePath.c_str());
            // TODO: Display error message to user
            cachedModBasePath = "";
            isCached = true;
            return cachedModBasePath;
        }

        cachedModBasePath = modBasePath;
        isCached = true;
        return cachedModBasePath;
    }
}
