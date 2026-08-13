#include "pch.h"
#include "common/gsc_loader.h"
#include "gsc_loader.h"

namespace t4
{
namespace mp
{
Detour Scr_AddSourceBuffer_Detour;

char *GSCLoader::Scr_AddSourceBuffer_Hook(scriptInstance_t inst, const char *filename, const char *extFilename,
                                          const char *codePos, bool archive)
{
    auto callOriginal = [&]()
    {
        return Scr_AddSourceBuffer_Detour.GetOriginal<decltype(Scr_AddSourceBuffer)>()(inst, filename, extFilename,
                                                                                       codePos, archive);
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

GSCLoader::GSCLoader()
{
    Scr_AddSourceBuffer_Detour = Detour(Scr_AddSourceBuffer, GSCLoader::Scr_AddSourceBuffer_Hook);
    Scr_AddSourceBuffer_Detour.Install();
}

GSCLoader::~GSCLoader()
{
    Scr_AddSourceBuffer_Detour.Remove();
}
} // namespace mp
} // namespace t4
