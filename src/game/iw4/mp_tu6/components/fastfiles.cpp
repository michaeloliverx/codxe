#include "pch.h"
#include "fastfiles.h"
#include "mods.h"
#include "common/usermaps.h"

namespace iw4
{
namespace mp_tu6
{
namespace
{
struct SysFile
{
    void *handle;
    int startOffset;
};

static_assert(sizeof(SysFile) == 8, "");

typedef void (*DB_LoadXAssets_t)(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
typedef SysFile (*Sys_CreateFile_t)(const char *dir, const char *filename);

auto DB_LoadXAssets = reinterpret_cast<DB_LoadXAssets_t>(0x821E1C60);
auto Sys_CreateFile = reinterpret_cast<Sys_CreateFile_t>(0x82319928);

Detour DB_LoadXAssets_Detour;
Detour Sys_CreateFile_Detour;

void DB_LoadXAssets_Hook(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
{
    bool initialBatch = false;
    bool containsMod = false;
    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        if (!zoneInfo[i].name)
            continue;
        initialBatch = initialBatch || _stricmp(zoneInfo[i].name, "common_mp") == 0;
        containsMod = containsMod || _stricmp(zoneInfo[i].name, "mod") == 0;
    }
    const std::string path = ModList::ResolvePath("mod.ff");
    if (initialBatch && !containsMod && zoneCount < 32 && !path.empty() && filesystem::FileExists(path.c_str()))
    {
        std::vector<XZoneInfo> zones(zoneInfo, zoneInfo + zoneCount);
        const XZoneInfo mod = {"mod", DB_ZONE_MOD, DB_ZONE_NONE};
        zones.push_back(mod);
        DB_LoadXAssets_Detour.GetOriginal<DB_LoadXAssets_t>()(zones.data(), static_cast<unsigned int>(zones.size()),
                                                              sync);
        return;
    }
    DB_LoadXAssets_Detour.GetOriginal<DB_LoadXAssets_t>()(zoneInfo, zoneCount, sync);
}

std::string ResolveFile(const char *filename)
{
    if (!filename || !*filename || std::strchr(filename, '/') || std::strchr(filename, '\\') ||
        std::strchr(filename, ':') || std::strstr(filename, ".."))
        return "";
    const size_t length = std::strlen(filename);
    const bool fastfile = length > 3 && _stricmp(filename + length - 3, ".ff") == 0;
    const bool imagefile =
        length > 4 && _strnicmp(filename, "imagefile", 9) == 0 && _stricmp(filename + length - 4, ".pak") == 0;
    if (!fastfile && !imagefile)
        return "";
    const std::string mod = ModList::ResolvePath(filename);
    if (!mod.empty() && filesystem::FileExists(mod.c_str()))
        return mod;
    if (fastfile)
    {
        const std::string map = usermaps::FastfilePath(filename);
        if (!map.empty() && filesystem::FileExists(map.c_str()))
            return map;
    }
    return Config::ResolveDataPath((std::string("zone/") + filename).c_str());
}

SysFile Sys_CreateFile_Hook(const char *dir, const char *filename)
{
    auto original = Sys_CreateFile_Detour.GetOriginal<Sys_CreateFile_t>();
    const std::string path = ResolveFile(filename);
    if (path.empty())
        return original(dir, filename);

    // Shared-root overrides use native streamed-file flags and start at offset zero.
    HANDLE file = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                              FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL);
    SysFile result = {file, 0};
    return result;
}

} // namespace

void fastfiles::ReloadModZone()
{
    XZoneInfo zones[2] = {{nullptr, DB_ZONE_NONE, DB_ZONE_MOD}, {"mod", DB_ZONE_MOD, DB_ZONE_NONE}};
    const std::string path = ModList::ResolvePath("mod.ff");
    DB_LoadXAssets(zones, !path.empty() && filesystem::FileExists(path.c_str()) ? 2 : 1, 1);
}

fastfiles::fastfiles()
{
    Sys_CreateFile_Detour = Detour(Sys_CreateFile, Sys_CreateFile_Hook);
    Sys_CreateFile_Detour.Install();

    DB_LoadXAssets_Detour = Detour(DB_LoadXAssets, DB_LoadXAssets_Hook);
    DB_LoadXAssets_Detour.Install();
}

fastfiles::~fastfiles()
{
    DB_LoadXAssets_Detour.Remove();
    Sys_CreateFile_Detour.Remove();
}
} // namespace mp_tu6
} // namespace iw4
