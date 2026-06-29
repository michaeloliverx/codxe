#pragma once

#include "core/context.h"

namespace codxe
{

bool RuntimeInit(PluginContext *ctx);
bool RuntimeLoadExecutable(PluginContext *ctx, uint32_t title_id, uint32_t timestamp);
void RuntimeUnloadTarget(PluginContext *ctx);
void RuntimeShutdown(PluginContext *ctx);

} // namespace codxe
