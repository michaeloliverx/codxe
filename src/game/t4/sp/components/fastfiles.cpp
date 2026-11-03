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
Detour BinkOpen_Detour;
typedef void *(*BinkOpen_t)(const char *name, unsigned int flags);

// TU7 sound pack layouts used by the native blocking header reader.
struct SoundPackIndex
{
    const char *name;
    DWORD header[5];
    const void *entries;
};

struct SoundPackSlot
{
    char name[64];
    DWORD handle;
    DWORD lastUse;
    DWORD size;
    BYTE packed;
    BYTE packedPadding[3];
    unsigned __int64 range;
    const SoundPackIndex *index;
    DWORD padding;
};

static_assert(sizeof(SoundPackIndex) == 28, "TU7 PackIndex layout");
static_assert(sizeof(SoundPackSlot) == 96, "TU7 sound pack slot layout");
static_assert(offsetof(SoundPackSlot, index) == 88, "TU7 pack registration field");

typedef int (*SoundReadBlocking_t)(SoundPackSlot *slot, DWORD offset, DWORD bytes, void *buffer);
Detour SoundReadBlocking_Detour;
DWORD usermapPackHandle = 0;

int SoundReadBlocking_Hook(SoundPackSlot *slot, DWORD offset, DWORD bytes, void *buffer)
{
    if (!slot || !usermapPackHandle || slot->handle != usermapPackHandle || offset || bytes != 20 || slot->packed)
    {
        return SoundReadBlocking_Detour.GetOriginal<SoundReadBlocking_t>()(slot, offset, bytes, buffer);
    }

    // Pack registration runs inside fastfile loading on the DB thread. Its
    // native ReadFileEx/SleepEx pair can dispatch outstanding fastfile APCs,
    // leaving the DB reader's later per-read SleepEx waits without a callback.
    // Use a private event and non-alertable wait ONLY for the 20-byte header;
    // sound payload reads retain the native async path on the sound thread.
    OVERLAPPED overlapped = {};
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!overlapped.hEvent)
    {
        const DWORD error = GetLastError();
        SetLastError(error);
        return 0;
    }

    const HANDLE handle = reinterpret_cast<HANDLE>(static_cast<ULONG_PTR>(slot->handle));
    DWORD transferred = 0;
    BOOL success = ReadFile(handle, buffer, bytes, &transferred, &overlapped);
    DWORD error = success ? ERROR_SUCCESS : GetLastError();
    if (success || error == ERROR_IO_PENDING)
    {
        success = GetOverlappedResult(handle, &overlapped, &transferred, TRUE);
        error = success ? ERROR_SUCCESS : GetLastError();
    }
    CloseHandle(overlapped.hEvent);
    success = success && transferred == bytes;
    if (!success && error == ERROR_SUCCESS)
        error = ERROR_HANDLE_EOF;
    SetLastError(error);
    return success ? 1 : 0;
}

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

bool RequiresLegacyUsermapSync(const char *zoneName)
{
    if (!IsSafeZoneName(zoneName) || EndsWithIgnoreCase(zoneName, "_load") || EndsWithIgnoreCase(zoneName, "_patch") ||
        !UsermapExists(zoneName))
    {
        return false;
    }

    const std::string loadZoneName = std::string(zoneName) + "_load";
    const std::string loadZonePath = GetUsermapFastfilePath(loadZoneName.c_str(), USERMAPS_DIRECTORY);
    return Config::ResolveDataPath(loadZonePath.c_str()).empty();
}

void SetActiveUsermap(const std::string &usermapName)
{
    if (_stricmp(activeUsermap, usermapName.c_str()) == 0)
        return;

    usermapPackHandle = 0;
    _snprintf_s(activeUsermap, sizeof(activeUsermap), _TRUNCATE, "%s", usermapName.c_str());
}

void ClearActiveUsermap()
{
    if (!activeUsermap[0])
        return;

    activeUsermap[0] = '\0';
    usermapPackHandle = 0;
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

std::string ResolveSoundPackPath(const char *filename)
{
    // Retail opens PackIndex::name under D:\; redirect only the active map's bundle.
    if (!filename || !activeUsermap[0] || _strnicmp(filename, "D:\\", 3) != 0 || !EndsWithIgnoreCase(filename, ".pak"))
    {
        return std::string();
    }

    const char *packName = filename + 3;
    const std::string expectedName = std::string(activeUsermap) + ".pak";
    if (!IsSafeZoneName(packName) || _stricmp(packName, expectedName.c_str()) != 0)
        return std::string();

    const std::string usermapDirectory = filesystem::JoinPath(USERMAPS_DIRECTORY, activeUsermap);
    const std::string packPath = filesystem::JoinPath(usermapDirectory.c_str(), packName);
    return Config::ResolveDataPathForGameFile(packPath.c_str());
}

// High mips are raw tiled GPU data, looked up by image name by the streamer.
// Only replace requests whose companion file exists in the active map.
std::string ResolveHighMipPath(const char *filename)
{
    if (!filename || !activeUsermap[0])
        return std::string();
    std::string request = filename;
    std::replace(request.begin(), request.end(), '/', '\\');
    std::transform(request.begin(), request.end(), request.begin(), ::tolower);
    const size_t marker = request.find("\\highmip\\");
    if (marker == std::string::npos)
        return std::string();
    const char *relative = filename + marker + 9;
    if (!IsSafeRelativePath(relative))
        return std::string();
    const std::string directory = filesystem::JoinPath(USERMAPS_DIRECTORY, activeUsermap);
    const std::string highmip = filesystem::JoinPath(directory.c_str(), "highmip");
    std::string resolved = Config::ResolveDataPathForGameFile(filesystem::JoinPath(highmip.c_str(), relative).c_str());
    // Retail discs may use h0/h1 partitions; usermaps also support a flat highmip directory.
    if (resolved.empty() && (_strnicmp(relative, "h0\\", 3) == 0 || _strnicmp(relative, "h1\\", 3) == 0 ||
                             _strnicmp(relative, "h0/", 3) == 0 || _strnicmp(relative, "h1/", 3) == 0))
        resolved = Config::ResolveDataPathForGameFile(filesystem::JoinPath(highmip.c_str(), relative + 3).c_str());
    return resolved;
}

void *BinkOpen_Hook(const char *name, unsigned int flags)
{
    std::string resolved;
    // BINKFROMMEMORY: name is a buffer, not a filename in this mode.
    if (name && !(flags & 0x04000000) && activeUsermap[0])
    {
        const char *basename = name;
        for (const char *cursor = name; *cursor; ++cursor)
            if (*cursor == '/' || *cursor == '\\')
                basename = cursor + 1;
        const std::string expected = std::string(activeUsermap) + "_load.bik";
        if (_stricmp(basename, expected.c_str()) == 0)
        {
            const std::string directory = filesystem::JoinPath(USERMAPS_DIRECTORY, activeUsermap);
            resolved =
                Config::ResolveDataPathForGameFile(filesystem::JoinPath(directory.c_str(), expected.c_str()).c_str());
        }
    }
    return BinkOpen_Detour.GetOriginal<BinkOpen_t>()(resolved.empty() ? name : resolved.c_str(), flags);
}

int Sys_CreateFile_Hook(const char *filename, int desiredAccess, int shareMode, int securityAttributes,
                        int creationDisposition, int flagsAndAttributes)
{
    bool redirectedSoundPack = false;
    std::string redirectedPath = ResolveFastfilePath(filename);
    if (!redirectedPath.empty())
        filename = redirectedPath.c_str();
    else
    {
        redirectedPath = ResolveHighMipPath(filename);
        if (redirectedPath.empty())
            redirectedPath = ResolveLooseSoundPath(filename);
        if (redirectedPath.empty())
        {
            redirectedPath = ResolveSoundPackPath(filename);
            redirectedSoundPack = !redirectedPath.empty();
        }
        if (!redirectedPath.empty())
            filename = redirectedPath.c_str();
    }

    // The native 20-byte pack header uses a stack buffer, which does not meet
    // HDD/USB unbuffered I/O requirements. Preserve asynchronous payload reads.
    if (redirectedSoundPack)
        flagsAndAttributes &= ~FILE_FLAG_NO_BUFFERING;

    const int handle = Sys_CreateFile_Detour.GetOriginal<Sys_CreateFile_t>()(
        filename, desiredAccess, shareMode, securityAttributes, creationDisposition, flagsAndAttributes);
    if (redirectedSoundPack && handle != -1)
        usermapPackHandle = static_cast<DWORD>(handle);
    return handle;
}

void DB_LoadXAssets_Hook(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
{
    UpdateActiveUsermap(zoneInfo, zoneCount);

    // Legacy monolithic maps need delayed asset clones linked before script compilation.
    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        if (RequiresLegacyUsermapSync(zoneInfo[i].name))
        {
            sync = 1;
            break;
        }
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
    usermapPackHandle = 0;
    expandedXAssetPoolCount = 0;

    DB_ReallocXAssetPool(ASSET_TYPE_MENULIST, 512);      // Stock: 128
    DB_ReallocXAssetPool(ASSET_TYPE_FX, 600);            // Stock: 400
    DB_ReallocXAssetPool(ASSET_TYPE_LOADED_SOUND, 2400); // Stock: 1600

    // TU7 BinkOpen also handles memory movies; the hook preserves that mode.
    BinkOpen_Detour = Detour(reinterpret_cast<void *>(0x82490BB0), BinkOpen_Hook);
    BinkOpen_Detour.Install();

    Sys_CreateFile_Detour = Detour(Sys_CreateFile, Sys_CreateFile_Hook);
    Sys_CreateFile_Detour.Install();

    DB_LoadXAssets_Detour = Detour(DB_LoadXAssets, DB_LoadXAssets_Hook);
    DB_LoadXAssets_Detour.Install();

    SoundReadBlocking_Detour = Detour(reinterpret_cast<void *>(0x823673D0), SoundReadBlocking_Hook);
    SoundReadBlocking_Detour.Install();
}

FastFiles::~FastFiles()
{
    BinkOpen_Detour.Remove();
    SoundReadBlocking_Detour.Remove();
    DB_LoadXAssets_Detour.Remove();
    Sys_CreateFile_Detour.Remove();

    RestoreExpandedXAssetPools();

    activeUsermap[0] = '\0';
    usermapPackHandle = 0;
}
} // namespace sp
} // namespace t4
