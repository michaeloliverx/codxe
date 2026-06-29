#pragma once

namespace codxe
{

struct PluginContext;

namespace features
{
namespace asset_overrides
{
namespace iw3_mp
{

bool Install(PluginContext *ctx);
void Shutdown(PluginContext *ctx);

} // namespace iw3_mp
} // namespace asset_overrides
} // namespace features
} // namespace codxe
