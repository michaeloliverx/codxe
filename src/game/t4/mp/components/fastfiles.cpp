#include "pch.h"
#include "fastfiles.h"

namespace t4
{
namespace mp
{
namespace
{
bool isSecure = true;

Detour CreateFileA_Detour;
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

    const std::string path = filesystem::JoinPath("codxe:\\zone", basename);
    return filesystem::FileExists(path.c_str()) ? path : std::string();
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

FastFiles::FastFiles()
{
    CreateFileA_Detour = Detour(GameCreateFileA, CreateFileA_Hook);
    CreateFileA_Detour.Install();

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
    DB_AuthLoad_InflateInit_Detour.Remove();
    DB_AuthLoad_Inflate_Detour.Remove();
    DB_AuthLoad_InflateEnd_Detour.Remove();
}
} // namespace mp
} // namespace t4
