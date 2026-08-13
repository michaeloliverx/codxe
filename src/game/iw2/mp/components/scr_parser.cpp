#include "pch.h"
#include "common/gsc_loader.h"
#include "scr_parser.h"

namespace iw2
{
namespace mp
{
Detour Scr_AddSourceBuffer_Detour;

char *Scr_AddSourceBuffer_Hook(const char *filename, const char *extFilename, const char *codePos, bool archive)
{
    auto callOriginal = [&]()
    {
        return Scr_AddSourceBuffer_Detour.GetOriginal<decltype(Scr_AddSourceBuffer)>()(filename, extFilename, codePos,
                                                                                       archive);
    };

    if (Config::dump_rawfile)
    {
        char *contents = callOriginal();
        gsc_loader::DumpSource(extFilename, contents);
        return contents;
    }

    char *contents = gsc_loader::TryLoadOverride(extFilename, Hunk_AllocateTempMemoryHighInternal);
    return contents ? contents : callOriginal();
}

scr_parser::scr_parser()
{
    Scr_AddSourceBuffer_Detour = Detour(Scr_AddSourceBuffer, Scr_AddSourceBuffer_Hook);
    Scr_AddSourceBuffer_Detour.Install();
}

scr_parser::~scr_parser()
{
    Scr_AddSourceBuffer_Detour.Remove();
}
} // namespace mp
} // namespace iw2
