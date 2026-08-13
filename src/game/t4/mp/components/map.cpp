#include "pch.h"
#include "map.h"

namespace t4
{
namespace mp
{
namespace
{
void OverrideMapEnts(MapEnts *mapEnts)
{
    const std::string fileName = map_ents::GetFileNameForAssetName(mapEnts->name);
    const std::string filePath = Config::ResolveModPath(fileName.c_str());
    if (filePath.empty())
    {
        return;
    }

    HANDLE file = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        return;
    }

    const DWORD fileSize = GetFileSize(file, nullptr);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0 || fileSize >= INT_MAX)
    {
        CloseHandle(file);
        return;
    }

    char *buffer = static_cast<char *>(Hunk_AllocLowAlign(fileSize + 1, 1));
    if (!buffer)
    {
        CloseHandle(file);
        return;
    }

    DWORD bytesRead = 0;
    if (!ReadFile(file, buffer, fileSize, &bytesRead, nullptr) || bytesRead != fileSize)
    {
        CloseHandle(file);
        return;
    }

    CloseHandle(file);
    buffer[fileSize] = '\0';

    mapEnts->entityString = buffer;
    mapEnts->numEntityChars = static_cast<int>(fileSize + 1);

    DbgPrint("[codxe][t4][mp] Loaded MapEnts override: %s (%u bytes at %p).\n", filePath.c_str(), fileSize, buffer);
}
} // namespace

Detour Load_clipMap_t_Detour;
void Load_clipMap_t_Hook(bool atStreamStart)
{
    // Call the original function to load the clip map
    Load_clipMap_t_Detour.GetOriginal<decltype(Load_clipMap_t)>()(atStreamStart);

    if (!varclipMap_t || !*varclipMap_t || !(*varclipMap_t)->name || !(*varclipMap_t)->mapEnts)
        return;

    auto mapEnts = (*varclipMap_t)->mapEnts;

    // Dump map entities if enabled
    if (Config::dump_map_ents)
    {
        const std::string dumpPath = map_ents::BuildPath(DUMP_DIR, mapEnts->name);
        if (!dumpPath.empty())
        {
            filesystem::WriteFileToDisk(dumpPath.c_str(), mapEnts->entityString, mapEnts->numEntityChars - 1);
            DbgPrint("Dumped map ents to: %s\n", dumpPath.c_str());
        }
    }

    OverrideMapEnts(mapEnts);
}

Map::Map()
{
    Load_clipMap_t_Detour = Detour(Load_clipMap_t, Load_clipMap_t_Hook);
    Load_clipMap_t_Detour.Install();
}

Map::~Map()
{
    Load_clipMap_t_Detour.Remove();
}
} // namespace mp
} // namespace t4
