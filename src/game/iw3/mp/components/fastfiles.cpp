#include "pch.h"
#include "fastfiles.h"

#include "common/string_utils.h"
#include "mods.h"

namespace iw3
{
namespace mp
{
namespace
{
const char *const MOD_ZONE = "mod";
const char *const MOD_FASTFILE = "mod.ff";
const char *const CODXE_COMMON_ZONE = "codxe_common_mp";
const char *const CODXE_UI_ZONE = "codxe_ui_mp";
const char *const CODXE_ZONE_DIRECTORY = "game:\\_codxe\\zone";
const char *const USERMAPS_DIRECTORY = "game:\\_codxe\\usermaps";
const char *const FASTFILE_EXTENSION = ".ff";

bool IsSafeZoneName(const char *name)
{
    if (!name || !*name || std::strcmp(name, ".") == 0 || std::strcmp(name, "..") == 0)
        return false;

    for (const char *cursor = name; *cursor; ++cursor)
    {
        const unsigned char c = static_cast<unsigned char>(*cursor);
        if (c < ' ' || c == '/' || c == '\\' || c == ':')
            return false;
    }

    return true;
}

std::string GetCodxeZoneFastfilePath(const char *zoneName)
{
    if (!IsSafeZoneName(zoneName))
        return std::string();

    const std::string filename = std::string(zoneName) + FASTFILE_EXTENSION;
    return filesystem::JoinPath(CODXE_ZONE_DIRECTORY, filename.c_str());
}

bool ContainsZone(const XZoneInfo *zoneInfo, unsigned int zoneCount, const char *name)
{
    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        if (zoneInfo[i].name && I_stricmp(zoneInfo[i].name, name) == 0)
            return true;
    }

    return false;
}

const XZoneInfo *FindZone(const XZoneInfo *zoneInfo, unsigned int zoneCount, const char *name)
{
    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        if (zoneInfo[i].name && I_stricmp(zoneInfo[i].name, name) == 0)
            return &zoneInfo[i];
    }

    return nullptr;
}

bool IsInitialZoneBatch(const XZoneInfo *zoneInfo, unsigned int zoneCount)
{
    return ContainsZone(zoneInfo, zoneCount, "code_post_gfx_mp") && ContainsZone(zoneInfo, zoneCount, "common_mp");
}

void DisableFastfileAuth()
{
    // DBX_AuthLoad_ValidateHash
    ppc::Nop(0x822B2994);
    ppc::Nop(0x822B2A34);
    ppc::Nop(0x822B2D2C);

    // DBX_AuthLoad_ValidateSignature
    ppc::Nop(0x822B2D44);
}

void DB_ReallocXAssetPool(XAssetType type, unsigned int newSize)
{
    void *poolEntry = malloc(newSize * DB_GetXAssetTypeSize(type));
    if (!poolEntry)
    {
        Com_Error(ERR_FATAL, "Failed to allocate expanded asset pool %d (%u entries)", type, newSize);
        return;
    }

    DB_XAssetPool[type] = poolEntry;
    g_poolSize[type] = newSize;
}

} // namespace

Detour FastFiles::DB_BuildOSPath_Detour;
Detour FastFiles::DB_LoadXAssets_Detour;

bool FastFiles::HasModFastfile()
{
    const std::string path = GetModFastfilePath();
    return !path.empty() && filesystem::FileExists(path.c_str());
}

std::string FastFiles::GetModFastfilePath()
{
    return ModList::ResolvePath(MOD_FASTFILE);
}

const char *FastFiles::GetUsermapsDirectory()
{
    return USERMAPS_DIRECTORY;
}

std::string FastFiles::GetUsermapFastfilePath(const char *zoneName)
{
    if (!IsSafeZoneName(zoneName) || I_strnicmp(zoneName, "mp_", 3) != 0)
        return std::string();

    std::string filename = zoneName;
    if (string_utils::EndsWith(filename, ".ff"))
        filename.erase(filename.length() - 3);

    std::string directory = filename;
    if (string_utils::EndsWith(directory, "_load"))
        directory.erase(directory.length() - 5);

    const std::string usermapDirectory = filesystem::JoinPath(USERMAPS_DIRECTORY, directory.c_str());
    return filesystem::JoinPath(usermapDirectory.c_str(), (filename + ".ff").c_str());
}

int FastFiles::DB_BuildOSPath_Hook(const char *zoneName, unsigned int size, char *filename)
{
    if (zoneName && I_stricmp(zoneName, MOD_ZONE) == 0)
    {
        const std::string path = GetModFastfilePath();
        if (!path.empty())
            return _snprintf_s(filename, size, _TRUNCATE, "%s", path.c_str());
    }

    const std::string usermapPath = GetUsermapFastfilePath(zoneName);
    if (!usermapPath.empty() && filesystem::FileExists(usermapPath.c_str()))
    {
        DbgPrint("[codxe][IW3][FastFiles] Redirecting usermap zone %s to %s\n", zoneName, usermapPath.c_str());
        return _snprintf_s(filename, size, _TRUNCATE, "%s", usermapPath.c_str());
    }

    const std::string codxeZonePath = GetCodxeZoneFastfilePath(zoneName);
    if (!codxeZonePath.empty() && filesystem::FileExists(codxeZonePath.c_str()))
    {
        DbgPrint("[codxe][IW3][FastFiles] Redirecting zone %s to %s\n", zoneName, codxeZonePath.c_str());
        return _snprintf_s(filename, size, _TRUNCATE, "%s", codxeZonePath.c_str());
    }

    return DB_BuildOSPath_Detour.GetOriginal<DB_BuildOSPath_t>()(zoneName, size, filename);
}

void FastFiles::DB_LoadXAssets_Hook(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
{
    auto original = DB_LoadXAssets_Detour.GetOriginal<DB_LoadXAssets_t>();
    const std::string codxeCommonFastfile = GetCodxeZoneFastfilePath(CODXE_COMMON_ZONE);
    const std::string codxeUiFastfile = GetCodxeZoneFastfilePath(CODXE_UI_ZONE);
    const XZoneInfo *stockCommonZone = FindZone(zoneInfo, zoneCount, "common_mp");
    const XZoneInfo *stockUiZone = FindZone(zoneInfo, zoneCount, "ui_mp");
    const bool injectCommon = filesystem::FileExists(codxeCommonFastfile.c_str()) && stockCommonZone &&
                              !ContainsZone(zoneInfo, zoneCount, CODXE_COMMON_ZONE);
    const bool injectUi = filesystem::FileExists(codxeUiFastfile.c_str()) && stockUiZone &&
                          !ContainsZone(zoneInfo, zoneCount, CODXE_UI_ZONE);
    const bool injectMod =
        HasModFastfile() && IsInitialZoneBatch(zoneInfo, zoneCount) && !ContainsZone(zoneInfo, zoneCount, MOD_ZONE);
    if (!injectCommon && !injectUi && !injectMod)
    {
        original(zoneInfo, zoneCount, sync);
        return;
    }

    std::vector<XZoneInfo> zones;
    zones.reserve(zoneCount + 3);

    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        zones.push_back(zoneInfo[i]);

        if (injectCommon && zoneInfo[i].name && I_stricmp(zoneInfo[i].name, "common_mp") == 0)
        {
            XZoneInfo commonZone = {CODXE_COMMON_ZONE, stockCommonZone->allocFlags, stockCommonZone->freeFlags};
            zones.push_back(commonZone);

            DbgPrint("[codxe][IW3][FastFiles] Loading common fastfile: %s\n", codxeCommonFastfile.c_str());
        }

        if (injectUi && zoneInfo[i].name && I_stricmp(zoneInfo[i].name, "ui_mp") == 0)
        {
            XZoneInfo uiZone = {CODXE_UI_ZONE, stockUiZone->allocFlags, stockUiZone->freeFlags};
            zones.push_back(uiZone);

            DbgPrint("[codxe][IW3][FastFiles] Loading UI fastfile: %s\n", codxeUiFastfile.c_str());
        }
    }

    if (injectMod)
    {
        XZoneInfo modZone = {MOD_ZONE, DB_ZONE_MOD, 0};
        zones.push_back(modZone);

        DbgPrint("[codxe][IW3][FastFiles] Loading startup mod fastfile: %s\n", GetModFastfilePath().c_str());
    }

    original(zones.data(), static_cast<unsigned int>(zones.size()), sync);
}

void FastFiles::ReloadModZone()
{
    XZoneInfo zones[2];
    unsigned int zoneCount = 1;

    // A null zone name with freeFlags is the engine's native free-only descriptor.
    zones[0].name = nullptr;
    zones[0].allocFlags = DB_ZONE_NONE;
    zones[0].freeFlags = DB_ZONE_MOD;

    const std::string path = GetModFastfilePath();
    if (!path.empty() && filesystem::FileExists(path.c_str()))
    {
        DbgPrint("[codxe][IW3][FastFiles] Loading mod fastfile: %s\n", path.c_str());
        zones[1].name = MOD_ZONE;
        zones[1].allocFlags = DB_ZONE_MOD;
        zones[1].freeFlags = 0;
        zoneCount = 2;
    }
    else
    {
        DbgPrint("[codxe][IW3][FastFiles] Active mod has no mod.ff; using loose files only\n");
    }

    DB_LoadXAssets(zones, zoneCount, 1);
    DbgPrint("[codxe][IW3][FastFiles] Mod zone reload complete\n");
}

FastFiles::FastFiles()
{
    DisableFastfileAuth();

    DB_ReallocXAssetPool(ASSET_TYPE_MENULIST, 256);         // Stock: 128
    DB_ReallocXAssetPool(ASSET_TYPE_MENU, 1024);            // Stock: 512
    DB_ReallocXAssetPool(ASSET_TYPE_LOCALIZE_ENTRY, 14000); // Stock: 6144
    DB_ReallocXAssetPool(ASSET_TYPE_FX, 600);               // Stock: 400

    DB_BuildOSPath_Detour = Detour(DB_BuildOSPath, DB_BuildOSPath_Hook);
    DB_BuildOSPath_Detour.Install();

    DB_LoadXAssets_Detour = Detour(DB_LoadXAssets, DB_LoadXAssets_Hook);
    DB_LoadXAssets_Detour.Install();
}

FastFiles::~FastFiles()
{
    DB_LoadXAssets_Detour.Remove();
    DB_BuildOSPath_Detour.Remove();
}
} // namespace mp
} // namespace iw3
