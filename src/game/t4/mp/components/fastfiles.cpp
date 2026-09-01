#include "pch.h"
#include "fastfiles.h"

namespace t4
{
namespace mp
{
namespace
{
bool isSecure = true;

Detour DB_AuthLoad_InflateInit_Detour;
Detour DB_AuthLoad_Inflate_Detour;
Detour DB_AuthLoad_InflateEnd_Detour;

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
    DB_AuthLoad_InflateInit_Detour.Remove();
    DB_AuthLoad_Inflate_Detour.Remove();
    DB_AuthLoad_InflateEnd_Detour.Remove();
}
} // namespace mp
} // namespace t4
