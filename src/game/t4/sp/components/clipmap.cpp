#include "clipmap.h"
#include "common.h"

namespace t4
{
namespace sp
{

Detour Load_clipMap_t_Detour;
void Load_clipMap_t_Hook(bool atStreamStart)
{
    // Call the original function to load the clip map
    Load_clipMap_t_Detour.GetOriginal<decltype(Load_clipMap_t)>()(atStreamStart);

    if (!varclipMap_t || !*varclipMap_t || !(*varclipMap_t)->name || !(*varclipMap_t)->mapEnts)
        return;

    Config config;
    LoadConfigFromFile(CONFIG_PATH, config);

    auto mapEnts = (*varclipMap_t)->mapEnts;

    // Dump map entities if enabled
    if (config.dump_map_ents)
    {
        std::string dumpPath = va("%s\\%s.ents", DUMP_DIR, mapEnts->name); // IW4x naming convention
        std::replace(dumpPath.begin(), dumpPath.end(), '/', '\\');
        filesystem::write_file_to_disk(dumpPath.c_str(), mapEnts->entityString, mapEnts->numEntityChars - 1);
        DbgPrint("Dumped map ents to: %s\n", dumpPath.c_str());
    }

    // Check for mod override
    std::string modBasePath = config.GetModBasePath();
    if (modBasePath.empty())
        return;

    // Build path to override file
    std::string overridePath = va("%s\\%s.ents", modBasePath.c_str(), mapEnts->name);
    std::replace(overridePath.begin(), overridePath.end(), '/', '\\');

    // Try to load override file
    std::string fileContent = filesystem::read_file_to_string(overridePath);
    if (fileContent.empty())
        return;

    // Allocate new buffer and copy content
    char *buffer = new char[fileContent.size() + 1];
    memcpy(buffer, fileContent.c_str(), fileContent.size());
    buffer[fileContent.size()] = '\0';

    // TODO: free old entity string if necessary?

    // Replace map entities
    mapEnts->entityString = buffer;
    mapEnts->numEntityChars = fileContent.size() + 1;

    DbgPrint("Loaded map ents override from: %s\n", overridePath.c_str());
}

clipmap::clipmap()
{
    Load_clipMap_t_Detour = Detour(Load_clipMap_t, Load_clipMap_t_Hook);
    Load_clipMap_t_Detour.Install();
}

clipmap::~clipmap()
{
    Load_clipMap_t_Detour.Remove();
}
} // namespace sp
} // namespace t4
