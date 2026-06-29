#pragma once

namespace codxe
{

struct PluginContext;

namespace features
{
namespace script_loading
{
namespace iw3_mp
{

bool Install(PluginContext *ctx);
void Shutdown(PluginContext *ctx);

} // namespace iw3_mp
} // namespace script_loading
} // namespace features
} // namespace codxe
