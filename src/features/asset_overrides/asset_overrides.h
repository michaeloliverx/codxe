#pragma once

#include "core/fixed_vector.h"
#include "hooks/detour.h"

namespace codxe
{

struct PluginContext;

namespace features
{
namespace asset_overrides
{

struct AssetLinkEvent
{
    void *asset_entry;
    int asset_type;
    void *asset_header;
    void *asset_data;
    int allow_override;
};

typedef void (*AssetLinkCallback)(PluginContext *ctx, AssetLinkEvent *event, void *user_data);

struct AssetLinkHandler
{
    AssetLinkCallback callback;
    void *user_data;
};

const size_t kMaxAssetLinkHandlers = 16;

struct AssetOverridesState
{
    Detour db_link_xasset_detour;
    bool db_link_xasset_hooked;
    FixedVector<AssetLinkHandler, kMaxAssetLinkHandlers> asset_link_handlers;
};

void AssetOverridesInit(AssetOverridesState *state);
bool AssetOverridesSubscribe(AssetOverridesState *state, AssetLinkCallback callback, void *user_data);
void AssetOverridesDispatch(PluginContext *ctx, AssetOverridesState *state, AssetLinkEvent *event);

bool InstallDbLinkXAssetHook(AssetOverridesState *state, void *source, const void *target);
void RemoveDbLinkXAssetHook(AssetOverridesState *state);

} // namespace asset_overrides
} // namespace features
} // namespace codxe
