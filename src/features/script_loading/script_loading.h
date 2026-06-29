#pragma once

#include "core/fixed_vector.h"
#include "hooks/detour.h"

namespace codxe
{

struct PluginContext;

namespace features
{
namespace script_loading
{

struct ScriptSourceRequest
{
    const char *filename;
    const char *ext_filename;
    const char *code_pos;
    bool archive;
    bool has_script_instance;
    int script_instance;
    char *replacement_source_buffer;
};

typedef void (*ScriptSourceCallback)(PluginContext *ctx, ScriptSourceRequest *request, void *user_data);

struct ScriptSourceHandler
{
    ScriptSourceCallback callback;
    void *user_data;
};

const size_t kMaxScriptSourceHandlers = 16;

struct ScriptLoadingState
{
    Detour scr_add_source_buffer_detour;
    bool scr_add_source_buffer_hooked;
    FixedVector<ScriptSourceHandler, kMaxScriptSourceHandlers> source_handlers;
};

void ScriptLoadingInit(ScriptLoadingState *state);
bool ScriptLoadingSubscribe(ScriptLoadingState *state, ScriptSourceCallback callback, void *user_data);
void ScriptLoadingDispatch(PluginContext *ctx, ScriptLoadingState *state, ScriptSourceRequest *request);

bool InstallScrAddSourceBufferHook(ScriptLoadingState *state, void *source, const void *target);
void RemoveScrAddSourceBufferHook(ScriptLoadingState *state);

} // namespace script_loading
} // namespace features
} // namespace codxe
