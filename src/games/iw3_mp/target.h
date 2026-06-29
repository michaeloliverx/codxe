#pragma once

#include "core/target.h"

namespace codxe
{
namespace games
{
namespace iw3_mp
{

extern const TargetDef kTarget;

bool InstallTarget(PluginContext *ctx);
void ShutdownTarget(PluginContext *ctx);
bool InstallLifecycleAdapter(PluginContext *ctx);

} // namespace iw3_mp
} // namespace games
} // namespace codxe
