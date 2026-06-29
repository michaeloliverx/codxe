#pragma once

#include "core/fixed_vector.h"

namespace codxe
{

struct PluginContext;

enum LifecycleEventId
{
    LIFECYCLE_DVAR_REGISTRATION,
    LIFECYCLE_VM_STARTUP,
    LIFECYCLE_VM_SHUTDOWN,
    LIFECYCLE_DRAW_FRAME,
    LIFECYCLE_EVENT_COUNT,
};

struct DvarRegistrationEvent
{
};

struct VmStartupEvent
{
    int script_instance;
};

struct VmShutdownEvent
{
    int script_instance;
    int system;
    int complete;
};

struct DrawFrameEvent
{
    int local_client_num;
};

typedef void (*LifecycleCallback)(PluginContext *ctx, void *event_args, void *user_data);

struct LifecycleHandler
{
    LifecycleCallback callback;
    void *user_data;
    bool once;
};

const size_t kMaxLifecycleHandlersPerEvent = 16;

struct LifecycleBus
{
    FixedVector<LifecycleHandler, kMaxLifecycleHandlersPerEvent> handlers[LIFECYCLE_EVENT_COUNT];
};

void LifecycleInit(LifecycleBus *bus);
bool LifecycleSubscribe(LifecycleBus *bus, LifecycleEventId event_id, LifecycleCallback callback, void *user_data,
                        bool once);
void LifecycleDispatch(PluginContext *ctx, LifecycleBus *bus, LifecycleEventId event_id, void *event_args);

} // namespace codxe
