#include "pch.h"
#include "common/gsc_loader.h"
#include "gsc_loader.h"
#include "mods.h"

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

    const std::string overridePath = ModList::ResolvePath(extFilename);
    char *contents =
        gsc_loader::TryLoadOverride(extFilename, overridePath.c_str(), Hunk_AllocateTempMemoryHighInternal);
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
