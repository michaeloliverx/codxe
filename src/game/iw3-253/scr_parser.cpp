#include <algorithm>

#include "..\..\detour.h"
#include "..\..\filesystem.h"
#include "..\..\xboxkrnl.h"

namespace iw3_253
{
    typedef char *(*Scr_AddSourceBuffer_t)(const char *filename, const char *extFilename, const char *codePos, bool archive);

    Scr_AddSourceBuffer_t Scr_AddSourceBuffer = reinterpret_cast<Scr_AddSourceBuffer_t>(0x8219DDA8);

    Detour Scr_AddSourceBuffer_Detour;

    char *Scr_AddSourceBuffer_Hook(const char *filename, const char *extFilename, const char *codePos, bool archive)
    {
        xbox::DbgPrint("Scr_AddSourceBuffer_Hook extFilename=%s \n", extFilename);

        std::string raw_file_path = "game:\\_iw3xe\\raw\\";
        raw_file_path += extFilename;
        std::replace(raw_file_path.begin(), raw_file_path.end(), '/', '\\'); // Replace forward slashes with backslashes
        if (filesystem::file_exists(raw_file_path))
        {
            xbox::DbgPrint("Found raw file: %s\n", raw_file_path.c_str());
            std::string new_contents = filesystem::read_file_to_string(raw_file_path);
            if (!new_contents.empty())
            {

                // Allocate new memory and copy the data
                size_t new_size = new_contents.size() + 1; // Include null terminator
                char *new_memory = static_cast<char *>(malloc(new_size));

                if (new_memory)
                {
                    memcpy(new_memory, new_contents.c_str(), new_size); // Copy with null terminator

                    xbox::DbgPrint("Replaced contents from file: %s\n", raw_file_path.c_str());
                    return new_memory;
                }
                else
                {
                    xbox::DbgPrint("Failed to allocate memory for contents replacement.\n");
                }
            }
        }

        return Scr_AddSourceBuffer_Detour.GetOriginal<decltype(Scr_AddSourceBuffer)>()(filename, extFilename, codePos, archive);
    }

    void init_scr_parser()
    {
        xbox::DbgPrint("Initializing Scr_AddSourceBuffer detour...\n");
        Scr_AddSourceBuffer_Detour = Detour(Scr_AddSourceBuffer, Scr_AddSourceBuffer_Hook);
        Scr_AddSourceBuffer_Detour.Install();
    }
}