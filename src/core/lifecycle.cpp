#include "core/lifecycle.h"

namespace codxe
{

void LifecycleInit(LifecycleBus *bus)
{
    if (bus == 0)
        return;

    for (int i = 0; i < LIFECYCLE_EVENT_COUNT; ++i)
    {
        bus->handlers[i].Clear();
    }
}

bool LifecycleSubscribe(LifecycleBus *bus, LifecycleEventId event_id, LifecycleCallback callback, void *user_data,
                        bool once)
{
    if (bus == 0 || callback == 0 || event_id < 0 || event_id >= LIFECYCLE_EVENT_COUNT)
        return false;

    LifecycleHandler handler = {};
    handler.callback = callback;
    handler.user_data = user_data;
    handler.once = once;

    return bus->handlers[event_id].PushBack(handler);
}

void LifecycleDispatch(PluginContext *ctx, LifecycleBus *bus, LifecycleEventId event_id, void *event_args)
{
    if (bus == 0 || event_id < 0 || event_id >= LIFECYCLE_EVENT_COUNT)
        return;

    FixedVector<LifecycleHandler, kMaxLifecycleHandlersPerEvent> &handlers = bus->handlers[event_id];

    size_t i = 0;
    while (i < handlers.count)
    {
        LifecycleHandler handler = handlers.items[i];
        handler.callback(ctx, event_args, handler.user_data);

        if (handler.once)
        {
            handlers.RemoveAt(i);
            continue;
        }

        ++i;
    }
}

} // namespace codxe
