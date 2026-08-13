#include "pch.h"
#include "map.h"

namespace t4
{
namespace mp
{
namespace
{
bool BuildMapEntsPath(char *path, size_t pathSize, const char *mapEntsName)
{
    const char *modBasePath = Config::GetModBasePath();
    if (!path || pathSize == 0 || !mapEntsName || mapEntsName[0] == '\0' || !modBasePath || modBasePath[0] == '\0')
    {
        return false;
    }

    const int written = _snprintf_s(path, pathSize, _TRUNCATE, "%s\\%s.ents", modBasePath, mapEntsName);
    path[pathSize - 1] = '\0';

    if (written < 0 || static_cast<size_t>(written) >= pathSize)
    {
        return false;
    }

    for (char *cursor = path; *cursor != '\0'; ++cursor)
    {
        if (*cursor == '/')
        {
            *cursor = '\\';
        }
    }

    return true;
}

void OverrideMapEnts(MapEnts *mapEnts)
{
    char filePath[MAX_PATH];
    if (!BuildMapEntsPath(filePath, sizeof(filePath), mapEnts->name))
    {
        return;
    }

    HANDLE file = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
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

    DbgPrint("[codxe][t4][mp] Loaded MapEnts override: %s (%u bytes at %p).\n", filePath, fileSize, buffer);
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
        std::string dumpPath = va("%s\\%s.ents", DUMP_DIR, mapEnts->name); // IW4x naming convention
        std::replace(dumpPath.begin(), dumpPath.end(), '/', '\\');
        filesystem::write_file_to_disk(dumpPath.c_str(), mapEnts->entityString, mapEnts->numEntityChars - 1);
        DbgPrint("Dumped map ents to: %s\n", dumpPath.c_str());
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
