#include "games/iw3_mp/target.h"

#include "features/asset_overrides/iw3_mp/asset_overrides.h"
#include "features/script_loading/iw3_mp/script_loading.h"
#include "games/iw3_mp/addresses.h"
#include "logging/log.h"

namespace codxe
{
namespace games
{
namespace iw3_mp
{

const Addresses kAddresses = {
    0x82236990, // Com_InitDvars
    0x82211FD0, // Scr_ShutdownSystem
    0x8231E6E0, // CG_DrawActive
    0x8229FC50, // DB_LinkXAsset
    0x822212C0, // Scr_AddSourceBuffer
};

const TargetDef kTarget = {
    "Call of Duty 4: Modern Warfare MP TU4", 0x415607E6, 0x4A78A577, InstallTarget, ShutdownTarget,
};

bool InstallTarget(PluginContext *ctx)
{
    Log(ctx, LOG_INFO, "installing target: %s", kTarget.name);
    if (!InstallLifecycleAdapter(ctx))
        return false;

    if (!features::script_loading::iw3_mp::Install(ctx))
        return false;

    if (!features::asset_overrides::iw3_mp::Install(ctx))
    {
        features::script_loading::iw3_mp::Shutdown(ctx);
        return false;
    }

    return true;
}

void ShutdownTarget(PluginContext *ctx)
{
    features::asset_overrides::iw3_mp::Shutdown(ctx);
    features::script_loading::iw3_mp::Shutdown(ctx);
    Log(ctx, LOG_INFO, "shutting down target: %s", kTarget.name);
}

} // namespace iw3_mp
} // namespace games
} // namespace codxe
