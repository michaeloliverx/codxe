#include "core/runtime.h"

#include "platform/xbox360.h"

namespace codxe
{

bool RuntimeInit(PluginContext *ctx)
{
    if (ctx == 0)
        return false;

    PluginContextInit(ctx);
    ctx->platform = DetectPlatform();
    Log(ctx, LOG_INFO, "platform: %s", GetPlatformName(ctx->platform.kind));
    return true;
}

bool RuntimeLoadExecutable(PluginContext *ctx, uint32_t title_id, uint32_t timestamp)
{
    if (ctx == 0)
        return false;

    RuntimeUnloadTarget(ctx);

    Log(ctx, LOG_INFO, "executable loaded: title_id=0x%08X timestamp=0x%08X", title_id, timestamp);

    ctx->target = FindTarget(title_id, timestamp);
    if (ctx->target == 0)
    {
        Log(ctx, LOG_WARN, "no v2 target matched current executable");
        return false;
    }

    if (!ctx->target->Install(ctx))
    {
        Log(ctx, LOG_ERROR, "target install failed: %s", ctx->target->name);
        ctx->target = 0;
        return false;
    }

    return true;
}

void RuntimeUnloadTarget(PluginContext *ctx)
{
    if (ctx == 0 || ctx->target == 0)
        return;

    if (ctx->target->Shutdown != 0)
        ctx->target->Shutdown(ctx);

    LifecycleInit(&ctx->lifecycle);
    features::asset_overrides::AssetOverridesInit(&ctx->asset_overrides);
    features::script_loading::ScriptLoadingInit(&ctx->script_loading);
    ctx->target = 0;
}

void RuntimeShutdown(PluginContext *ctx)
{
    if (ctx == 0)
        return;

    RuntimeUnloadTarget(ctx);
}

} // namespace codxe
