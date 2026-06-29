#include "features/script_loading/script_loading.h"

namespace codxe
{
namespace features
{
namespace script_loading
{

void ScriptLoadingInit(ScriptLoadingState *state)
{
    if (state == 0)
        return;

    DetourInit(&state->scr_add_source_buffer_detour, 0, 0);
    state->scr_add_source_buffer_hooked = false;
    state->source_handlers.Clear();
}

bool ScriptLoadingSubscribe(ScriptLoadingState *state, ScriptSourceCallback callback, void *user_data)
{
    if (state == 0 || callback == 0)
        return false;

    ScriptSourceHandler handler = {};
    handler.callback = callback;
    handler.user_data = user_data;

    return state->source_handlers.PushBack(handler);
}

void ScriptLoadingDispatch(PluginContext *ctx, ScriptLoadingState *state, ScriptSourceRequest *request)
{
    if (state == 0 || request == 0)
        return;

    FixedVector<ScriptSourceHandler, kMaxScriptSourceHandlers> &handlers = state->source_handlers;

    for (size_t i = 0; i < handlers.count; ++i)
    {
        ScriptSourceHandler handler = handlers.items[i];
        handler.callback(ctx, request, handler.user_data);

        if (request->replacement_source_buffer != 0)
            return;
    }
}

bool InstallScrAddSourceBufferHook(ScriptLoadingState *state, void *source, const void *target)
{
    if (state == 0 || source == 0 || target == 0 || state->scr_add_source_buffer_hooked)
        return false;

    DetourInit(&state->scr_add_source_buffer_detour, source, target);
    state->scr_add_source_buffer_hooked = DetourInstall(&state->scr_add_source_buffer_detour);
    return state->scr_add_source_buffer_hooked;
}

void RemoveScrAddSourceBufferHook(ScriptLoadingState *state)
{
    if (state == 0 || !state->scr_add_source_buffer_hooked)
        return;

    DetourRemove(&state->scr_add_source_buffer_detour);
    state->scr_add_source_buffer_hooked = false;
}

} // namespace script_loading
} // namespace features
} // namespace codxe
