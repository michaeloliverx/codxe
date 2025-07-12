#include "gsc_loader.h"
#include "common.h"

namespace t4
{
    namespace mp
    {
        Detour Scr_AddSourceBuffer_Detour;

        char *GSCLoader::Scr_AddSourceBuffer_Hook(scriptInstance_t a1, const char *filename, const char *extFilename, const char *codePos, bool archive)
        {
            // filename - the file name without extension, e.g., "example" - paths are unix style e.g. "maps/mp/gametypes/example"
            // extFilename - the file name with extension, e.g., "example.gsc" - paths are unix style e.g. // "maps/mp/gametypes/example.gsc"

            std::string base = GetModBasePath();
            if (base.empty())
            {
                return Scr_AddSourceBuffer_Detour.GetOriginal<decltype(Scr_AddSourceBuffer)>()(a1, filename, extFilename, codePos, archive);
            }

            std::string fullPath = base + "\\";
            fullPath += extFilename; // Use the full path with extension
            std::replace(fullPath.begin(), fullPath.end(), '/', '\\');

            DbgPrint("Full path: %s\n", fullPath.c_str());

            FILE *file = fopen(fullPath.c_str(), "rb");
            if (file)
            {
                DbgPrint("GSCLoader: Found shadowed script file: %s\n", fullPath.c_str());
                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                rewind(file);

                // Allocate temp buffer from hunk
                char *buffer = (char *)Hunk_AllocateTempMemoryHighInternal(file_size + 1);
                if (buffer)
                {
                    fread(buffer, 1, file_size, file);
                    buffer[file_size] = '\0'; // Null terminate

                    fclose(file);
                    return buffer; // Return overwritten script
                }

                fclose(file);
            }

            return Scr_AddSourceBuffer_Detour.GetOriginal<decltype(Scr_AddSourceBuffer)>()(a1, filename, extFilename, codePos, archive);
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
