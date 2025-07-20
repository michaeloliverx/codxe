#include "gsc_loader.h"
#include "common.h"

namespace t4
{
    namespace mp
    {
        Detour Scr_AddSourceBuffer_Detour;

        char *GSCLoader::Scr_AddSourceBuffer_Hook(scriptInstance_t a1, const char *filename, const char *extFilename, const char *codePos, bool archive)
        {
            auto callOriginal = [&]()
            {
                return Scr_AddSourceBuffer_Detour.GetOriginal<decltype(Scr_AddSourceBuffer)>()(a1, filename, extFilename, codePos, archive);
            };

            Config config;
            LoadConfigFromFile(CONFIG_PATH, config);

            if (config.dump_raw)
            {
                auto contents = callOriginal();
                // Dump the script to a file
                std::string dumpPath = std::string(DUMP_DIR) + "\\" + extFilename;
                std::replace(dumpPath.begin(), dumpPath.end(), '/', '\\');
                filesystem::write_file_to_disk(dumpPath.c_str(), contents, std::strlen(contents));
                DbgPrint("GSCLoader: Dumped script to %s\n", dumpPath.c_str());
                return contents;
            }

            // Check if mod is active
            std::string modBasePath = config.GetModBasePath();
            if (modBasePath.empty())
                return callOriginal();

            // Build full path to override file
            std::string overridePath = modBasePath + "\\" + extFilename;
            std::replace(overridePath.begin(), overridePath.end(), '/', '\\');

            // Try to load override file
            std::string fileContent = filesystem::read_file_to_string(overridePath);
            if (fileContent.empty())
                return callOriginal();

            // Allocate buffer using game's memory allocator
            char *buffer = (char *)Hunk_AllocateTempMemoryHighInternal(fileContent.size() + 1);
            if (!buffer)
                return callOriginal();

            // Copy content and null terminate
            memcpy(buffer, fileContent.c_str(), fileContent.size());
            buffer[fileContent.size()] = '\0';

            DbgPrint("GSCLoader: Loaded override script: %s\n", overridePath.c_str());
            return buffer;
        }

        GSCLoader::GSCLoader()
        {
            Scr_AddSourceBuffer_Detour = Detour(Scr_AddSourceBuffer, GSCLoader::Scr_AddSourceBuffer_Hook);
            Scr_AddSourceBuffer_Detour.Install();
        }

        GSCLoader::~GSCLoader()
        {
            Scr_AddSourceBuffer_Detour.Remove();
        }
    }
}
