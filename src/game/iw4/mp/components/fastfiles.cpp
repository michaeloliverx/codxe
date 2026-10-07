#include "pch.h"
#include "fastfiles.h"

namespace iw4
{
namespace mp
{
namespace
{
const char *const CODXE_PATCH_ZONE = "codxe_patch_mp";
const char *const CODXE_UI_ZONE = "codxe_ui_mp";
const char *const CODXE_IMAGEFILE_FILENAME = "imagefile5.pak";
const char *const FASTFILE_EXTENSION = ".ff";
const unsigned int MAX_ZONE_COUNT = 32;

struct XZoneInfo
{
    const char *name;
    int allocFlags;
    int freeFlags;
};

static_assert(sizeof(XZoneInfo) == 12, "XZoneInfo size mismatch");

typedef void (*DB_LoadXAssets_t)(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
typedef int (*Sys_CreateFile_t)(const char *dir, const char *filename);

auto DB_LoadXAssets = reinterpret_cast<DB_LoadXAssets_t>(0x8219B298);
auto Sys_CreateFile = reinterpret_cast<Sys_CreateFile_t>(0x822AA590);

Detour DB_LoadXAssets_Detour;
Detour Sys_CreateFile_Detour;

const XZoneInfo *FindZone(const XZoneInfo *zoneInfo, unsigned int zoneCount, const char *name)
{
    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        if (zoneInfo[i].name && std::strcmp(zoneInfo[i].name, name) == 0)
            return &zoneInfo[i];
    }

    return nullptr;
}

bool IsSafeFastfileName(const char *filename)
{
    if (!filename)
        return false;

    const std::size_t filenameLength = std::strlen(filename);
    const std::size_t extensionLength = std::strlen(FASTFILE_EXTENSION);

    if (filenameLength <= extensionLength ||
        std::strcmp(filename + filenameLength - extensionLength, FASTFILE_EXTENSION) != 0)
    {
        return false;
    }

    for (std::size_t i = 0; i < filenameLength - extensionLength; ++i)
    {
        const char c = filename[i];
        const bool isLetter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        const bool isDigit = c >= '0' && c <= '9';
        if (!isLetter && !isDigit && c != '_' && c != '-')
            return false;
    }

    return true;
}

void DB_LoadXAssets_Hook(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
{
    auto original = DB_LoadXAssets_Detour.GetOriginal<DB_LoadXAssets_t>();
    const bool patchFileExists = !Config::ResolveDataPath("zone\\codxe_patch_mp.ff").empty();
    const bool uiFileExists = !Config::ResolveDataPath("zone\\codxe_ui_mp.ff").empty();
    const XZoneInfo *stockPatchZone = FindZone(zoneInfo, zoneCount, "patch_mp");
    const XZoneInfo *stockUiZone = FindZone(zoneInfo, zoneCount, "ui_mp");
    const bool injectPatch = patchFileExists && stockPatchZone;
    const bool injectUi = uiFileExists && stockUiZone;
    const unsigned int injectionCount = (injectPatch ? 1 : 0) + (injectUi ? 1 : 0);
    const bool hasCapacity = zoneCount <= MAX_ZONE_COUNT && injectionCount <= MAX_ZONE_COUNT - zoneCount;

    if (injectionCount == 0 || !hasCapacity)
    {
        original(zoneInfo, zoneCount, sync);
        return;
    }

    XZoneInfo zones[MAX_ZONE_COUNT];
    unsigned int outputZoneCount = 0;

    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        if (injectUi && zoneInfo[i].name && std::strcmp(zoneInfo[i].name, "ui_mp") == 0)
        {
            zones[outputZoneCount].name = CODXE_UI_ZONE;
            zones[outputZoneCount].allocFlags = stockUiZone->allocFlags;
            zones[outputZoneCount].freeFlags = stockUiZone->freeFlags;
            ++outputZoneCount;
        }

        zones[outputZoneCount] = zoneInfo[i];
        ++outputZoneCount;
    }

    if (injectPatch)
    {
        zones[outputZoneCount].name = CODXE_PATCH_ZONE;
        zones[outputZoneCount].allocFlags = stockPatchZone->allocFlags;
        zones[outputZoneCount].freeFlags = stockPatchZone->freeFlags;
        ++outputZoneCount;
    }

    original(zones, outputZoneCount, sync);
}

int Sys_CreateFile_Hook(const char *dir, const char *filename)
{
    auto original = Sys_CreateFile_Detour.GetOriginal<Sys_CreateFile_t>();

    if (filename && (std::strcmp(filename, CODXE_IMAGEFILE_FILENAME) == 0 || IsSafeFastfileName(filename)))
    {
        const std::string path = Config::ResolveDataPath(filesystem::JoinPath("zone", filename).c_str());
        // Sys_CreateFile prepends the title directory, so pass a title-relative filename.
        if (path.compare(0, 6, "game:\\") == 0)
            return original(dir, path.substr(6).c_str());

        // External roots use a device link directly, with the stock async/unbuffered read flags.
        if (!path.empty())
            return reinterpret_cast<int>(CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                                                     OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING,
                                                     nullptr));
    }

    return original(dir, filename);
}
} // namespace

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
} // namespace mp
} // namespace iw4
