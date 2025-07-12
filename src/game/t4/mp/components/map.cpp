#include "map.h"
#include "common.h"

bool DUMP_MAP_ENTS = true;

namespace t4
{
    namespace mp
    {
        Detour Load_clipMap_t_Detour;
        void Load_clipMap_t_Hook(bool atStreamStart)
        {
            // Call the original function to load the clip map
            Load_clipMap_t_Detour.GetOriginal<decltype(Load_clipMap_t)>()(atStreamStart);

            if (!varclipMap_t || !*varclipMap_t || !(*varclipMap_t)->name || !(*varclipMap_t)->mapEnts)
                return;

            auto mapEnts = (*varclipMap_t)->mapEnts;

            if (DUMP_MAP_ENTS)
            {
                // Write stock map ents to disk
                std::string ents_dump_path = va("%s\\%s.ents", t4::DUMP_DIR, mapEnts->name); // IW4x naming convention
                DbgPrint("Dumping map ents to file: %s\n", ents_dump_path.c_str());
                std::replace(ents_dump_path.begin(), ents_dump_path.end(), '/', '\\');                                      // Replace forward slashes with backslashes
                filesystem::write_file_to_disk(ents_dump_path.c_str(), mapEnts->entityString, mapEnts->numEntityChars - 1); // Exclude the null terminator
            }

            std::string base = GetModBasePath();
            if (base.empty())
            {
                DbgPrint("No active mod found, skipping custom map ents loading.\n");
                return;
            }

            std::string ents_read_path = base + "\\";
            ents_read_path += mapEnts->name; // Use the map name without extension
            ents_read_path += ".ents";       // Append the .ents extension
            DbgPrint("Reading map ents from file: %s\n", ents_read_path.c_str());
            std::replace(ents_read_path.begin(), ents_read_path.end(), '/', '\\');

            FILE *file = fopen(ents_read_path.c_str(), "rb");
            if (!file)
                return;

            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            rewind(file);

            char *buffer = new char[file_size + 1]; // +1 for null terminator
            fread(buffer, 1, file_size, file);
            buffer[file_size] = '\0'; // Null-terminate the string
            fclose(file);

            DbgPrint("Loaded map ents from file: %s\n", ents_read_path.c_str());

            // TODO: Check for memory leaks on the old varclipMap_t fields still existing
            (*varclipMap_t)->mapEnts->entityString = buffer;          // Assign the new entityString
            (*varclipMap_t)->mapEnts->numEntityChars = file_size + 1; // Update the number of characters (including null terminator)
        }

        Map::Map()
        {
            DbgPrint("Map initialized\n");

            Load_clipMap_t_Detour = Detour(Load_clipMap_t, Load_clipMap_t_Hook);
            Load_clipMap_t_Detour.Install();
        }

        Map::~Map()
        {
            DbgPrint("Map shutdown\n");

            Load_clipMap_t_Detour.Remove();
        }
    }
}
