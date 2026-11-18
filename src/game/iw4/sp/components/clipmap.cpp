#include "pch.h"
#include "clipmap.h"

namespace iw4
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

    auto mapEnts = (*varclipMap_t)->mapEnts;

    const std::string fileName = map_ents::GetFileNameForAssetName(mapEnts->name);
    const std::string overridePath = Config::ResolveModPath(fileName.c_str());
    if (overridePath.empty())
    {
        return;
    }

    // Try to load override file
    std::string fileContent = filesystem::ReadFileToString(overridePath);
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
} // namespace sp
} // namespace iw4
