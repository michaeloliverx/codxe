#pragma once

#include "core/lifecycle.h"
#include "core/target.h"
#include "features/asset_overrides/asset_overrides.h"
#include "features/script_loading/script_loading.h"
#include "logging/log.h"
#include "platform/platform.h"

namespace codxe
{

struct PluginContext
{
    PlatformInfo platform;
    const TargetDef *target;
    LogState log;
    LifecycleBus lifecycle;
    features::asset_overrides::AssetOverridesState asset_overrides;
    features::script_loading::ScriptLoadingState script_loading;
};

void PluginContextInit(PluginContext *ctx);

} // namespace codxe
