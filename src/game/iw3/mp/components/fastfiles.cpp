#include "pch.h"
#include "fastfiles.h"

#include "common/string_utils.h"
#include "mods.h"
#include "common/usermaps.h"

namespace iw3
{
namespace mp
{
namespace
{
const char *const MOD_ZONE = "mod";
const char *const MOD_FASTFILE = "mod.ff";
const char *const CODXE_ZONE_DIRECTORY = "codxe:\\zone";
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

std::string GetZoneNameFromFastfileRequest(const char *filename)
{
    if (!filename || !*filename)
        return std::string();

    const char *basename = filename;
    for (const char *cursor = filename; *cursor; ++cursor)
    {
        if (*cursor == '\\' || *cursor == '/')
            basename = cursor + 1;
    }

    const size_t length = std::strlen(basename);
    if (length <= 3 || I_stricmp(basename + length - 3, FASTFILE_EXTENSION) != 0)
        return std::string();

    const std::string zoneName(basename, length - 3);
    return IsSafeZoneName(zoneName.c_str()) ? zoneName : std::string();
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

Detour FastFiles::CreateFileA_Detour;
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
    return usermaps::GetDirectory();
}

std::string FastFiles::GetUsermapFastfilePath(const char *zoneName)
{
    return usermaps::FastfilePath(zoneName);
}

std::string FastFiles::ResolveFastfilePath(const char *filename)
{
    const std::string zoneName = GetZoneNameFromFastfileRequest(filename);
    if (zoneName.empty())
        return std::string();

    if (I_stricmp(zoneName.c_str(), MOD_ZONE) == 0)
    {
        const std::string path = GetModFastfilePath();
        if (!path.empty())
            return path;
    }

    const std::string usermapPath = GetUsermapFastfilePath(zoneName.c_str());
    if (!usermapPath.empty() && filesystem::FileExists(usermapPath.c_str()))
        return usermapPath;

    const std::string codxeZonePath = GetCodxeZoneFastfilePath(zoneName.c_str());
    if (!codxeZonePath.empty() && filesystem::FileExists(codxeZonePath.c_str()))
        return codxeZonePath;

    return std::string();
}

HANDLE FastFiles::CreateFileA_Hook(const char *filename, DWORD desiredAccess, DWORD shareMode,
                                   LPSECURITY_ATTRIBUTES securityAttributes, DWORD creationDisposition,
                                   DWORD flagsAndAttributes, HANDLE templateFile)
{
    std::string replacement;
    if (creationDisposition == OPEN_EXISTING && (desiredAccess & GENERIC_READ) && !(desiredAccess & GENERIC_WRITE))
        replacement = ResolveFastfilePath(filename);

    if (!replacement.empty())
    {
        // TU4 opens D:\patch_mp.ff directly, bypassing DB_BuildOSPath.
        // Redirect the open so stock zone names, flags and lifetimes remain intact.
        DbgPrint("[codxe][IW3][FastFiles] Redirecting fastfile %s to %s\n", filename, replacement.c_str());
        filename = replacement.c_str();
    }

    return CreateFileA_Detour.GetOriginal<GameCreateFileA_t>()(filename, desiredAccess, shareMode, securityAttributes,
                                                               creationDisposition, flagsAndAttributes, templateFile);
}

void FastFiles::DB_LoadXAssets_Hook(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
{
    auto original = DB_LoadXAssets_Detour.GetOriginal<DB_LoadXAssets_t>();
    const bool injectMod =
        HasModFastfile() && IsInitialZoneBatch(zoneInfo, zoneCount) && !ContainsZone(zoneInfo, zoneCount, MOD_ZONE);
    if (!injectMod)
    {
        original(zoneInfo, zoneCount, sync);
        return;
    }

    std::vector<XZoneInfo> zones(zoneInfo, zoneInfo + zoneCount);
    XZoneInfo modZone = {MOD_ZONE, DB_ZONE_MOD, 0};
    zones.push_back(modZone);

    DbgPrint("[codxe][IW3][FastFiles] Loading startup mod fastfile: %s\n", GetModFastfilePath().c_str());

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

    CreateFileA_Detour = Detour(GameCreateFileA, CreateFileA_Hook);
    CreateFileA_Detour.Install();

    DB_LoadXAssets_Detour = Detour(DB_LoadXAssets, DB_LoadXAssets_Hook);
    DB_LoadXAssets_Detour.Install();
}

FastFiles::~FastFiles()
{
    DB_LoadXAssets_Detour.Remove();
    CreateFileA_Detour.Remove();
}
} // namespace mp
} // namespace iw3
