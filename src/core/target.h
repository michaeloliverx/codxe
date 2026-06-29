#pragma once

#include <cstdint>

namespace codxe
{

struct PluginContext;

struct TargetDef
{
    const char *name;
    uint32_t title_id;
    uint32_t timestamp;
    bool (*Install)(PluginContext *ctx);
    void (*Shutdown)(PluginContext *ctx);
};

const TargetDef *FindTarget(uint32_t title_id, uint32_t timestamp);

} // namespace codxe
