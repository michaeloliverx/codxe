#include <algorithm>

#include "sp_main.h"
#include "sp_structs.h"

#include "../detour.h"
#include "../filesystem.h"
#include "../xboxkrnl.h"

namespace sp
{
    Scr_ReadFile_FastFile_t Scr_ReadFile_FastFile = reinterpret_cast<Scr_ReadFile_FastFile_t>(0x821C5978);

    Detour Scr_ReadFile_FastFile_Detour;

    char *Scr_ReadFile_FastFile_Hook(const char *filename, const char *extFilename, const char *codePos, bool archive)
    {
        xbox::DbgPrint("Scr_ReadFile_FastFile_Hook extFilename=%s \n", extFilename);

        auto contents = Scr_ReadFile_FastFile_Detour.GetOriginal<decltype(&Scr_ReadFile_FastFile_Hook)>()(filename, extFilename, codePos, archive);

        // Dump the file to disk if it exists
        // It might not exist if it's not a stock file
        if (contents != nullptr)
        {
            // Dump the file to disk
            std::string file_path = "game:\\dump\\";
            file_path += extFilename;
            std::replace(file_path.begin(), file_path.end(), '/', '\\'); // Replace forward slashes with backslashes
            filesystem::write_file_to_disk(file_path.c_str(), contents, strlen(contents));
        }

        std::string raw_file_path = "game:\\raw\\";
        raw_file_path += extFilename;
        std::replace(raw_file_path.begin(), raw_file_path.end(), '/', '\\'); // Replace forward slashes with backslashes
        if (filesystem::file_exists(raw_file_path))
        {
            xbox::DbgPrint("Found raw file: %s\n", raw_file_path.c_str());
            // return ReadFileContents(raw_file_path);
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

        return contents;
    }

    void init()
    {
        xbox::DbgPrint("Initializing SP logic...");

        Scr_ReadFile_FastFile_Detour = Detour(Scr_ReadFile_FastFile, Scr_ReadFile_FastFile_Hook);
        Scr_ReadFile_FastFile_Detour.Install();
    }
}
