#include "pch.h"
#include "fastfiles.h"
#include "mods.h"

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

    const std::string path = filesystem::JoinPath("codxe:\\zone", basename);
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
    if (creationDisposition == OPEN_EXISTING && (desiredAccess & GENERIC_READ) && !(desiredAccess & GENERIC_WRITE))
        replacement = ResolveFastfilePath(filename);

    if (!replacement.empty())
    {
        // TU7 opens patch_mp and patch_ui_mp on D: before trying the retail path.
        // Redirect the open while retaining the stock zone priority and lifetime.
        DbgPrint("[codxe][T4][FastFiles] Redirecting fastfile %s to %s\n", filename, replacement.c_str());
        filename = replacement.c_str();
    }

    return CreateFileA_Detour.GetOriginal<GameCreateFileA_t>()(filename, desiredAccess, shareMode, securityAttributes,
                                                               creationDisposition, flagsAndAttributes, templateFile);
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
    CreateFileA_Detour.Remove();
    DB_LoadXAssets_Detour.Remove();
    DB_AuthLoad_InflateInit_Detour.Remove();
    DB_AuthLoad_Inflate_Detour.Remove();
    DB_AuthLoad_InflateEnd_Detour.Remove();
}
} // namespace mp
} // namespace t4
