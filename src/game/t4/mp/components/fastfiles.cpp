#include "pch.h"
#include "fastfiles.h"
#include "mods.h"
#include "common/usermaps.h"

namespace t4
{
namespace mp
{
namespace
{
bool isSecure = true;
// TU7 MP uses 0x20 for patch_mp; its separate mod allocation is 0x40.
const int DB_ZONE_MOD = 0x40;

Detour CreateFileA_Detour;
Detour DB_LoadXAssets_Detour;
Detour DB_AuthLoad_InflateInit_Detour;
Detour DB_AuthLoad_Inflate_Detour;
Detour DB_AuthLoad_InflateEnd_Detour;

// TU7 MP pack slots are 96 bytes; only these fields are needed for header reads.
struct SoundPackSlot
{
    char name[64];
    DWORD handle;
    BYTE padding[8];
    BYTE packed;
    BYTE reserved[19];
};
static_assert(sizeof(SoundPackSlot) == 96, "");
static_assert(offsetof(SoundPackSlot, handle) == 64, "");
static_assert(offsetof(SoundPackSlot, packed) == 76, "");

const DWORD SOUND_PACK_HEADER_SIZE = 20;
typedef int (*SoundReadBlocking_t)(SoundPackSlot *slot, DWORD offset, DWORD bytes, void *buffer);
auto SoundReadBlocking = reinterpret_cast<SoundReadBlocking_t>(0x8237E490);
Detour SoundReadBlocking_Detour;
DWORD usermapPackHandle = 0;

int SoundReadBlocking_Hook(SoundPackSlot *slot, DWORD offset, DWORD bytes, void *buffer)
{
    auto original = SoundReadBlocking_Detour.GetOriginal<SoundReadBlocking_t>();
    if (!slot || !usermapPackHandle || slot->handle != usermapPackHandle || offset || bytes != SOUND_PACK_HEADER_SIZE ||
        slot->packed)
        return original(slot, offset, bytes, buffer);

    const std::string expected = std::string(Dvar_GetString("mapname")) + ".pak";
    if (_stricmp(slot->name, expected.c_str()) != 0)
        return original(slot, offset, bytes, buffer);

    // As in SP, keep DB-thread pack registration out of the native alertable
    // ReadFileEx/SleepEx path, which can consume fastfile completion callbacks.
    OVERLAPPED overlapped = {};
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!overlapped.hEvent)
        return 0;

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

std::string GetActiveUsermapDirectory()
{
    const char *map = Dvar_GetString("mapname");
    const std::string fastfile = usermaps::FastfilePath(map);
    if (fastfile.empty() || !filesystem::FileExists(fastfile.c_str()))
        return std::string();
    return filesystem::JoinPath(usermaps::GetDirectory(), map);
}

std::string ResolveFastfilePath(const char *filename)
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
    if (length <= 3 || _stricmp(basename + length - 3, ".ff") != 0)
        return std::string();

    for (const char *cursor = basename; *cursor; ++cursor)
    {
        if (static_cast<unsigned char>(*cursor) < ' ' || *cursor == ':')
            return std::string();
    }

    if (_stricmp(basename, "mod.ff") == 0)
    {
        const std::string modPath = ModList::ResolvePath("mod.ff");
        return !modPath.empty() && filesystem::FileExists(modPath.c_str()) ? modPath : std::string();
    }

    const std::string usermap = usermaps::FastfilePath(basename);
    if (!usermap.empty() && filesystem::FileExists(usermap.c_str()))
        return usermap;

    const std::string path = filesystem::JoinPath("codxe:\\zone", basename);
    return filesystem::FileExists(path.c_str()) ? path : std::string();
}

std::string ResolveHighMipPath(const char *filename)
{
    if (!filename)
        return std::string();

    std::string request = filename;
    std::replace(request.begin(), request.end(), '/', '\\');
    const std::string lower = arena::Lower(request);
    const size_t marker = lower.find("\\highmip\\");
    if (marker == std::string::npos)
        return std::string();

    std::string image = request.substr(marker + 9);
    // Retail highmips may use h0/h1 folders; map packages use a flat directory.
    if (_strnicmp(image.c_str(), "h0\\", 3) == 0 || _strnicmp(image.c_str(), "h1\\", 3) == 0)
        image.erase(0, 3);
    if (image.size() <= 3 || _stricmp(image.c_str() + image.size() - 3, ".hi") != 0 ||
        image.find_first_of("\\:") != std::string::npos)
        return std::string();

    // Use the running map, independently of the map highlighted in the menus.
    const std::string directory = GetActiveUsermapDirectory();
    if (directory.empty())
        return std::string();
    const std::string highmip = filesystem::JoinPath(directory.c_str(), "highmip");
    const std::string path = filesystem::JoinPath(highmip.c_str(), image.c_str());
    return filesystem::FileExists(path.c_str()) ? path : std::string();
}

std::string ResolveSoundPackPath(const char *filename)
{
    if (!filename || _strnicmp(filename, "D:\\", 3) != 0)
        return std::string();
    const std::string expected = std::string(Dvar_GetString("mapname")) + ".pak";
    if (_stricmp(filename + 3, expected.c_str()) != 0)
        return std::string();
    const std::string directory = GetActiveUsermapDirectory();
    if (directory.empty())
        return std::string();
    const std::string path = filesystem::JoinPath(directory.c_str(), expected.c_str());
    return filesystem::FileExists(path.c_str()) ? path : std::string();
}

std::string ResolveLooseSoundPath(const char *filename)
{
    if (!filename)
        return std::string();
    std::string request = filename;
    std::replace(request.begin(), request.end(), '/', '\\');
    const char *prefix = "D:\\sounds\\";
    if (_strnicmp(request.c_str(), prefix, std::strlen(prefix)) != 0)
        return std::string();
    const std::string relative = request.substr(std::strlen(prefix));
    if (relative.empty() || relative.find(':') != std::string::npos)
        return std::string();
    for (size_t start = 0; start <= relative.size();)
    {
        const size_t end = relative.find('\\', start);
        const std::string part = relative.substr(start, end == std::string::npos ? end : end - start);
        if (part.empty() || part == "." || part == "..")
            return std::string();
        if (end == std::string::npos)
            break;
        start = end + 1;
    }
    const std::string directory = GetActiveUsermapDirectory();
    if (directory.empty())
        return std::string();
    const std::string sounds = filesystem::JoinPath(directory.c_str(), "sounds");
    const std::string path = filesystem::JoinPath(sounds.c_str(), relative.c_str());
    return filesystem::FileExists(path.c_str()) ? path : std::string();
}

void DB_LoadXAssets_Hook(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
{
    bool initialBatch = false;
    bool containsMod = false;
    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        if (zoneInfo[i].name)
        {
            initialBatch = initialBatch || _stricmp(zoneInfo[i].name, "common_mp") == 0;
            containsMod = containsMod || _stricmp(zoneInfo[i].name, "mod") == 0;
        }
    }

    const std::string path = ModList::ResolvePath("mod.ff");
    if (initialBatch && !containsMod && !path.empty() && filesystem::FileExists(path.c_str()))
    {
        std::vector<XZoneInfo> zones(zoneInfo, zoneInfo + zoneCount);
        XZoneInfo mod = {"mod", DB_ZONE_MOD, 0};
        zones.push_back(mod);
        DB_LoadXAssets_Detour.GetOriginal<decltype(DB_LoadXAssets)>()(zones.data(),
                                                                      static_cast<unsigned int>(zones.size()), sync);
        return;
    }

    DB_LoadXAssets_Detour.GetOriginal<decltype(DB_LoadXAssets)>()(zoneInfo, zoneCount, sync);
}

HANDLE CreateFileA_Hook(const char *filename, DWORD desiredAccess, DWORD shareMode,
                        LPSECURITY_ATTRIBUTES securityAttributes, DWORD creationDisposition, DWORD flagsAndAttributes,
                        HANDLE templateFile)
{
    std::string replacement;
    bool soundPack = false;
    if (creationDisposition == OPEN_EXISTING && (desiredAccess & GENERIC_READ) && !(desiredAccess & GENERIC_WRITE))
    {
        replacement = ResolveFastfilePath(filename);
        if (replacement.empty())
            replacement = ResolveHighMipPath(filename);
        if (replacement.empty())
            replacement = ResolveLooseSoundPath(filename);
        if (replacement.empty())
        {
            replacement = ResolveSoundPackPath(filename);
            soundPack = !replacement.empty();
        }
    }

    if (!replacement.empty())
    {
        // TU7 opens patch_mp and patch_ui_mp on D: before trying the retail path.
        // Redirect the open while retaining the stock zone priority and lifetime.
        DbgPrint("[codxe][T4][FastFiles] Redirecting %s to %s\n", filename, replacement.c_str());
        filename = replacement.c_str();
    }

    // The 20-byte header's stack buffer cannot satisfy unbuffered HDD/USB I/O.
    // Keep overlapped payload streaming and all stock file flags unchanged.
    if (soundPack)
        flagsAndAttributes &= ~FILE_FLAG_NO_BUFFERING;
    const HANDLE handle = CreateFileA_Detour.GetOriginal<GameCreateFileA_t>()(
        filename, desiredAccess, shareMode, securityAttributes, creationDisposition, flagsAndAttributes, templateFile);
    if (soundPack)
        usermapPackHandle =
            handle != INVALID_HANDLE_VALUE ? static_cast<DWORD>(reinterpret_cast<ULONG_PTR>(handle)) : 0;
    return handle;
}

int DB_AuthLoad_InflateInit_Hook(z_stream_s *stream, bool secure, const char *filename)
{
    isSecure = secure;

    const int result =
        secure ? DB_AuthLoad_InflateInit_Detour.GetOriginal<DB_AuthLoad_InflateInit_t>()(stream, secure, filename)
               : inflateInit_(stream, "1.1.4", sizeof(z_stream_s));

    if (result)
    {
        R_ShowDirtyDiscError();
    }

    return result;
}

int DB_AuthLoad_Inflate_Hook(z_stream_s *stream, int flush)
{
    if (isSecure)
    {
        return DB_AuthLoad_Inflate_Detour.GetOriginal<DB_AuthLoad_Inflate_t>()(stream, flush);
    }

    return inflate(stream, flush);
}

void DB_AuthLoad_InflateEnd_Hook(z_stream_s *stream)
{
    if (isSecure)
    {
        DB_AuthLoad_InflateEnd_Detour.GetOriginal<DB_AuthLoad_InflateEnd_t>()(stream);
        return;
    }

    inflateEnd(stream);
}
} // namespace

void FastFiles::ReloadModZone()
{
    // Free only the previous mod. Common, TU patch and frontend assets stay resident.
    XZoneInfo zones[2] = {{nullptr, 0, DB_ZONE_MOD}, {"mod", DB_ZONE_MOD, 0}};
    const std::string path = ModList::ResolvePath("mod.ff");
    const bool hasFastfile = !path.empty() && filesystem::FileExists(path.c_str());
    DB_LoadXAssets(zones, hasFastfile ? 2 : 1, 1);
}

FastFiles::FastFiles()
{
    usermapPackHandle = 0;
    SoundReadBlocking_Detour = Detour(SoundReadBlocking, SoundReadBlocking_Hook);
    SoundReadBlocking_Detour.Install();

    CreateFileA_Detour = Detour(GameCreateFileA, CreateFileA_Hook);
    CreateFileA_Detour.Install();

    DB_LoadXAssets_Detour = Detour(DB_LoadXAssets, DB_LoadXAssets_Hook);
    DB_LoadXAssets_Detour.Install();

    DB_AuthLoad_InflateInit_Detour = Detour(DB_AuthLoad_InflateInit, DB_AuthLoad_InflateInit_Hook);
    DB_AuthLoad_InflateInit_Detour.Install();

    DB_AuthLoad_Inflate_Detour = Detour(DB_AuthLoad_Inflate, DB_AuthLoad_Inflate_Hook);
    DB_AuthLoad_Inflate_Detour.Install();

    DB_AuthLoad_InflateEnd_Detour = Detour(DB_AuthLoad_InflateEnd, DB_AuthLoad_InflateEnd_Hook);
    DB_AuthLoad_InflateEnd_Detour.Install();

    // DB_LoadXFileInternal rejects IWffu100 after successfully setting up its
    // decompressor. The hooks above provide the missing unsigned path, so skip
    // that final "file not supported" dirty-disc call.
    ppc::Nop(0x821D89CC);
}

FastFiles::~FastFiles()
{
    SoundReadBlocking_Detour.Remove();
    CreateFileA_Detour.Remove();
    DB_LoadXAssets_Detour.Remove();
    DB_AuthLoad_InflateInit_Detour.Remove();
    DB_AuthLoad_Inflate_Detour.Remove();
    DB_AuthLoad_InflateEnd_Detour.Remove();
    usermapPackHandle = 0;
}
} // namespace mp
} // namespace t4
