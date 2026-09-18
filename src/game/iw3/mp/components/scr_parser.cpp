#include "pch.h"
#include "common/gsc_loader.h"
#include "mods.h"
#include "scr_parser.h"

namespace iw3
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

    const std::string overridePath = ModList::ResolvePath(extFilename);
    char *contents =
        gsc_loader::TryLoadOverride(extFilename, overridePath.c_str(), Hunk_AllocateTempMemoryHighInternal);
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
} // namespace iw3
