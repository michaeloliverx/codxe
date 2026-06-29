#include "core/context.h"

namespace codxe
{

void PluginContextInit(PluginContext *ctx)
{
    if (ctx == 0)
        return;

    PlatformInit(&ctx->platform);
    ctx->target = 0;
    LogInit(&ctx->log);
    LifecycleInit(&ctx->lifecycle);
    features::asset_overrides::AssetOverridesInit(&ctx->asset_overrides);
    features::script_loading::ScriptLoadingInit(&ctx->script_loading);
}

} // namespace codxe
