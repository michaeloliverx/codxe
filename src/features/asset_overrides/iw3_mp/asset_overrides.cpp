#include "features/asset_overrides/iw3_mp/asset_overrides.h"

#include "core/context.h"
#include "features/asset_overrides/asset_overrides.h"
#include "games/iw3_mp/addresses.h"
#include "games/iw3_mp/structs.h"
#include "hooks/detour.h"
#include "logging/log.h"

namespace codxe
{
namespace features
{
namespace asset_overrides
{
namespace iw3_mp
{
namespace
{

PluginContext *s_context = 0;

games::iw3_mp::DB_LinkXAsset_t GetOriginalDbLinkXAsset()
{
    if (s_context == 0)
        return 0;

    return DetourGetOriginal<games::iw3_mp::DB_LinkXAsset_t>(&s_context->asset_overrides.db_link_xasset_detour);
}

games::iw3_mp::XAssetEntry *DB_LinkXAsset_Hook(games::iw3_mp::XAssetEntry *new_entry, int allow_override)
{
    if (s_context != 0 && new_entry != 0)
    {
        AssetLinkEvent event = {};
        event.asset_entry = new_entry;
        event.asset_type = static_cast<int>(new_entry->asset.type);
        event.asset_header = &new_entry->asset.header;
        event.asset_data = new_entry->asset.header.data;
        event.allow_override = allow_override;

        AssetOverridesDispatch(s_context, &s_context->asset_overrides, &event);
    }

    games::iw3_mp::DB_LinkXAsset_t original = GetOriginalDbLinkXAsset();
    return original != 0 ? original(new_entry, allow_override) : 0;
}

} // namespace

bool Install(PluginContext *ctx)
{
    if (ctx == 0)
        return false;

    s_context = ctx;

    if (!InstallDbLinkXAssetHook(&ctx->asset_overrides,
                                 reinterpret_cast<void *>(games::iw3_mp::kAddresses.db_link_xasset),
                                 DB_LinkXAsset_Hook))
    {
        Log(ctx, LOG_ERROR, "failed to install IW3 MP DB_LinkXAsset hook");
        s_context = 0;
        return false;
    }

    Log(ctx, LOG_INFO, "IW3 MP DB_LinkXAsset hook installed at 0x%08X", games::iw3_mp::kAddresses.db_link_xasset);
    return true;
}

void Shutdown(PluginContext *ctx)
{
    if (ctx == 0)
        return;

    RemoveDbLinkXAssetHook(&ctx->asset_overrides);
    s_context = 0;
}

} // namespace iw3_mp
} // namespace asset_overrides
} // namespace features
} // namespace codxe
