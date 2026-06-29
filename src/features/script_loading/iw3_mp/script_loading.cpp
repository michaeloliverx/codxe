#include "features/script_loading/iw3_mp/script_loading.h"

#include "core/context.h"
#include "features/script_loading/script_loading.h"
#include "games/iw3_mp/addresses.h"
#include "hooks/detour.h"
#include "logging/log.h"

namespace codxe
{
namespace features
{
namespace script_loading
{
namespace iw3_mp
{
namespace
{

PluginContext *s_context = 0;

games::iw3_mp::Scr_AddSourceBuffer_t GetOriginalScrAddSourceBuffer()
{
    if (s_context == 0)
        return 0;

    return DetourGetOriginal<games::iw3_mp::Scr_AddSourceBuffer_t>(
        &s_context->script_loading.scr_add_source_buffer_detour);
}

char *Scr_AddSourceBuffer_Hook(const char *filename, const char *ext_filename, const char *code_pos, bool archive)
{
    if (s_context != 0)
    {
        ScriptSourceRequest request = {};
        request.filename = filename;
        request.ext_filename = ext_filename;
        request.code_pos = code_pos;
        request.archive = archive;
        request.has_script_instance = false;
        request.script_instance = 0;
        request.replacement_source_buffer = 0;

        ScriptLoadingDispatch(s_context, &s_context->script_loading, &request);
        if (request.replacement_source_buffer != 0)
            return request.replacement_source_buffer;
    }

    games::iw3_mp::Scr_AddSourceBuffer_t original = GetOriginalScrAddSourceBuffer();
    return original != 0 ? original(filename, ext_filename, code_pos, archive) : 0;
}

} // namespace

bool Install(PluginContext *ctx)
{
    if (ctx == 0)
        return false;

    s_context = ctx;

    if (!InstallScrAddSourceBufferHook(&ctx->script_loading,
                                       reinterpret_cast<void *>(games::iw3_mp::kAddresses.scr_add_source_buffer),
                                       Scr_AddSourceBuffer_Hook))
    {
        Log(ctx, LOG_ERROR, "failed to install IW3 MP Scr_AddSourceBuffer hook");
        s_context = 0;
        return false;
    }

    Log(ctx, LOG_INFO, "IW3 MP Scr_AddSourceBuffer hook installed at 0x%08X",
        games::iw3_mp::kAddresses.scr_add_source_buffer);
    return true;
}

void Shutdown(PluginContext *ctx)
{
    if (ctx == 0)
        return;

    RemoveScrAddSourceBufferHook(&ctx->script_loading);
    s_context = 0;
}

} // namespace iw3_mp
} // namespace script_loading
} // namespace features
} // namespace codxe
