#include "pch.h"
#include "fastfiles.h"

namespace t4
{
namespace sp
{
namespace
{
const char *const CODXE_ZONE_DIRECTORY = "zone";
const char *const USERMAPS_DIRECTORY = "usermaps";
const char *const SOUND_REQUEST_PREFIX = "D:\\sounds\\";

char activeUsermap[64] = "";

Detour DB_LoadXAssets_Detour;
Detour Sys_CreateFile_Detour;

struct ExpandedXAssetPool
{
    XAssetType type;
    void *originalPool;
    int originalSize;
    void *expandedPool;
};

ExpandedXAssetPool expandedXAssetPools[3] = {};
unsigned int expandedXAssetPoolCount = 0;

bool EndsWithIgnoreCase(const std::string &value, const char *suffix)
{
    const size_t suffixLength = std::strlen(suffix);
    return value.length() >= suffixLength && _stricmp(value.c_str() + value.length() - suffixLength, suffix) == 0;
}

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

bool IsSafeRelativePath(const char *path)
{
    if (!path || !*path || *path == '\\' || *path == '/')
        return false;

    const char *component = path;
    for (const char *cursor = path;; ++cursor)
    {
        const unsigned char c = static_cast<unsigned char>(*cursor);
        if (c == ':' || (c && c < ' '))
            return false;

        if (!c || c == '\\' || c == '/')
        {
            const size_t componentLength = static_cast<size_t>(cursor - component);
            if (!componentLength || (componentLength == 1 && component[0] == '.') ||
                (componentLength == 2 && component[0] == '.' && component[1] == '.'))
            {
                return false;
            }

            if (!c)
                return true;

            component = cursor + 1;
        }
    }
}

std::string GetUsermapName(const char *zoneName)
{
    if (!IsSafeZoneName(zoneName))
        return std::string();

    std::string usermapName = zoneName;
    if (EndsWithIgnoreCase(usermapName, "_load"))
        usermapName.erase(usermapName.length() - 5);
    else if (EndsWithIgnoreCase(usermapName, "_patch"))
        usermapName.erase(usermapName.length() - 6);

    return usermapName;
}

std::string GetUsermapFastfilePath(const char *zoneName, const char *rootDirectory)
{
    const std::string usermapName = GetUsermapName(zoneName);
    if (usermapName.empty())
        return std::string();

    const std::string usermapDirectory = filesystem::JoinPath(rootDirectory, usermapName.c_str());
    const std::string filename = std::string(zoneName) + ".ff";
    return filesystem::JoinPath(usermapDirectory.c_str(), filename.c_str());
}

std::string GetZoneNameFromFastfileRequest(const char *filename)
{
    static const char extension[] = ".ff";

    if (!filename || !*filename)
        return std::string();

    const char *basename = filename;
    for (const char *cursor = filename; *cursor; ++cursor)
    {
        if (*cursor == '\\' || *cursor == '/')
            basename = cursor + 1;
    }

    std::string zoneName = basename;
    if (!EndsWithIgnoreCase(zoneName, extension))
        return std::string();

    zoneName.erase(zoneName.length() - (sizeof(extension) - 1));
    return IsSafeZoneName(zoneName.c_str()) ? zoneName : std::string();
}

std::string GetZoneFastfilePath(const char *zoneName, const char *rootDirectory)
{
    if (!IsSafeZoneName(zoneName))
        return std::string();

    return filesystem::JoinPath(rootDirectory, (std::string(zoneName) + ".ff").c_str());
}

std::string ResolveFastfilePath(const char *filename)
{
    const std::string zoneName = GetZoneNameFromFastfileRequest(filename);
    if (zoneName.empty())
        return std::string();

    // Only redirect the file open. The database keeps the requested zone's stock name, flags and lifetime.
    // Future active-mod replacements belong here, before usermap and global replacements.

    const std::string usermapPath = GetUsermapFastfilePath(zoneName.c_str(), USERMAPS_DIRECTORY);
    if (!usermapPath.empty())
    {
        const std::string resolved = Config::ResolveDataPathForGameFile(usermapPath.c_str());
        if (!resolved.empty())
            return resolved;
    }

    const std::string codxeZonePath = GetZoneFastfilePath(zoneName.c_str(), CODXE_ZONE_DIRECTORY);
    if (!codxeZonePath.empty())
        return Config::ResolveDataPathForGameFile(codxeZonePath.c_str());

    return std::string();
}

bool UsermapExists(const char *usermapName)
{
    const std::string path = GetUsermapFastfilePath(usermapName, USERMAPS_DIRECTORY);
    return !path.empty() && !Config::ResolveDataPath(path.c_str()).empty();
}

void SetActiveUsermap(const std::string &usermapName)
{
    if (_stricmp(activeUsermap, usermapName.c_str()) == 0)
        return;

    _snprintf_s(activeUsermap, sizeof(activeUsermap), _TRUNCATE, "%s", usermapName.c_str());
}

void ClearActiveUsermap()
{
    if (!activeUsermap[0])
        return;

    activeUsermap[0] = '\0';
}

void UpdateActiveUsermap(const XZoneInfo *zoneInfo, unsigned int zoneCount)
{
    bool containsLoadscreenZone = false;

    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        const std::string usermapName = GetUsermapName(zoneInfo[i].name);
        if (usermapName.empty())
            continue;

        if (EndsWithIgnoreCase(zoneInfo[i].name, "_load"))
            containsLoadscreenZone = true;

        if (UsermapExists(usermapName.c_str()))
        {
            SetActiveUsermap(usermapName);
            return;
        }
    }

    // A new stock loadscreen marks the transition away from the previous custom map.
    if (containsLoadscreenZone)
        ClearActiveUsermap();
}

bool RequiresPriorityOverride(const char *name)
{
    // Loadscreen zones use a temporary link-time priority below; changing their allocation group breaks unloading.
    return name && !EndsWithIgnoreCase(name, "_load") && UsermapExists(name);
}

std::string ResolveLooseSoundPath(const char *filename)
{
    if (!filename || !activeUsermap[0] ||
        _strnicmp(filename, SOUND_REQUEST_PREFIX, std::strlen(SOUND_REQUEST_PREFIX)) != 0)
    {
        return std::string();
    }

    const char *relativeSoundPath = filename + std::strlen(SOUND_REQUEST_PREFIX);
    if (!IsSafeRelativePath(relativeSoundPath))
        return std::string();

    const std::string usermapDirectory = filesystem::JoinPath(USERMAPS_DIRECTORY, activeUsermap);
    const std::string soundDirectory = filesystem::JoinPath(usermapDirectory.c_str(), "sounds");
    const std::string soundPath = filesystem::JoinPath(soundDirectory.c_str(), relativeSoundPath);
    return Config::ResolveDataPathForGameFile(soundPath.c_str());
}

int Sys_CreateFile_Hook(const char *filename, int desiredAccess, int shareMode, int securityAttributes,
                        int creationDisposition, int flagsAndAttributes)
{
    std::string redirectedPath = ResolveFastfilePath(filename);
    if (!redirectedPath.empty())
        filename = redirectedPath.c_str();
    else
    {
        redirectedPath = ResolveLooseSoundPath(filename);
        if (!redirectedPath.empty())
            filename = redirectedPath.c_str();
    }

    return Sys_CreateFile_Detour.GetOriginal<Sys_CreateFile_t>()(filename, desiredAccess, shareMode, securityAttributes,
                                                                 creationDisposition, flagsAndAttributes);
}

void DB_LoadXAssets_Hook(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
{
    UpdateActiveUsermap(zoneInfo, zoneCount);

    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        if (!RequiresPriorityOverride(zoneInfo[i].name))
            continue;

        // Preserve the map's normal allocation group while giving its intentional overrides priority.
        zoneInfo[i].allocFlags |= DB_ZONE_DEV;

        // Complete delayed asset clones before script compilation starts.
        sync = 1;
    }

    DB_LoadXAssets_Detour.GetOriginal<DB_LoadXAssets_t>()(zoneInfo, zoneCount, sync);
}

void DB_ReallocXAssetPool(XAssetType type, unsigned int newSize)
{
    if (*g_assetPoolsInitialized)
    {
        DbgPrint("[codxe][T4 SP][FastFiles] Cannot expand initialized %s asset pool\n", g_assetNames[type]);
        return;
    }

    const unsigned int oldSize = static_cast<unsigned int>(g_poolSize[type]);
    if (newSize <= oldSize)
        return;

    void *pool = malloc(newSize * DB_GetXAssetTypeSize(type));
    if (!pool)
    {
        DbgPrint("[codxe][T4 SP][FastFiles] Failed to expand %s asset pool from %u to %u entries\n", g_assetNames[type],
                 oldSize, newSize);
        return;
    }

    if (expandedXAssetPoolCount == ARRAYSIZE(expandedXAssetPools))
    {
        DbgPrint("[codxe][T4 SP][FastFiles] Cannot track expanded %s asset pool\n", g_assetNames[type]);
        free(pool);
        return;
    }

    ExpandedXAssetPool &expandedPool = expandedXAssetPools[expandedXAssetPoolCount++];
    expandedPool.type = type;
    expandedPool.originalPool = DB_XAssetPool[type];
    expandedPool.originalSize = g_poolSize[type];
    expandedPool.expandedPool = pool;

    DB_XAssetPool[type] = expandedPool.expandedPool;
    g_poolSize[type] = newSize;
}

void RestoreExpandedXAssetPools()
{
    while (expandedXAssetPoolCount)
    {
        ExpandedXAssetPool &expandedPool = expandedXAssetPools[--expandedXAssetPoolCount];
        DB_XAssetPool[expandedPool.type] = expandedPool.originalPool;
        g_poolSize[expandedPool.type] = expandedPool.originalSize;
        free(expandedPool.expandedPool);
        memset(&expandedPool, 0, sizeof(expandedPool));
    }
}
} // namespace

FastFiles::FastFiles()
{
    // The DLL remains resident across title launches, so explicitly reset title-lifetime state.
    activeUsermap[0] = '\0';
    expandedXAssetPoolCount = 0;

    DB_ReallocXAssetPool(ASSET_TYPE_MENULIST, 192); // Stock: 128
    DB_ReallocXAssetPool(ASSET_TYPE_MENU, 800);     // Stock: 600
    DB_ReallocXAssetPool(ASSET_TYPE_FX, 600);       // Stock: 400

    Sys_CreateFile_Detour = Detour(Sys_CreateFile, Sys_CreateFile_Hook);
    Sys_CreateFile_Detour.Install();

    DB_LoadXAssets_Detour = Detour(DB_LoadXAssets, DB_LoadXAssets_Hook);
    DB_LoadXAssets_Detour.Install();
}

FastFiles::~FastFiles()
{
    DB_LoadXAssets_Detour.Remove();
    Sys_CreateFile_Detour.Remove();

    RestoreExpandedXAssetPools();

    activeUsermap[0] = '\0';
}
} // namespace sp
} // namespace t4
