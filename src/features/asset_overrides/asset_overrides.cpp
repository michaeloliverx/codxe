#include "features/asset_overrides/asset_overrides.h"

namespace codxe
{
namespace features
{
namespace asset_overrides
{

void AssetOverridesInit(AssetOverridesState *state)
{
    if (state == 0)
        return;

    DetourInit(&state->db_link_xasset_detour, 0, 0);
    state->db_link_xasset_hooked = false;
    state->asset_link_handlers.Clear();
}

bool AssetOverridesSubscribe(AssetOverridesState *state, AssetLinkCallback callback, void *user_data)
{
    if (state == 0 || callback == 0)
        return false;

    AssetLinkHandler handler = {};
    handler.callback = callback;
    handler.user_data = user_data;

    return state->asset_link_handlers.PushBack(handler);
}

void AssetOverridesDispatch(PluginContext *ctx, AssetOverridesState *state, AssetLinkEvent *event)
{
    if (state == 0 || event == 0)
        return;

    FixedVector<AssetLinkHandler, kMaxAssetLinkHandlers> &handlers = state->asset_link_handlers;

    for (size_t i = 0; i < handlers.count; ++i)
    {
        AssetLinkHandler handler = handlers.items[i];
        handler.callback(ctx, event, handler.user_data);
    }
}

bool InstallDbLinkXAssetHook(AssetOverridesState *state, void *source, const void *target)
{
    if (state == 0 || source == 0 || target == 0 || state->db_link_xasset_hooked)
        return false;

    DetourInit(&state->db_link_xasset_detour, source, target);
    state->db_link_xasset_hooked = DetourInstall(&state->db_link_xasset_detour);
    return state->db_link_xasset_hooked;
}

void RemoveDbLinkXAssetHook(AssetOverridesState *state)
{
    if (state == 0 || !state->db_link_xasset_hooked)
        return;

    DetourRemove(&state->db_link_xasset_detour);
    state->db_link_xasset_hooked = false;
}

} // namespace asset_overrides
} // namespace features
} // namespace codxe
