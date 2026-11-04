#include "pch.h"
#include "notifications.h"

namespace
{
const DWORD STARTUP_DELAY_MS = 3000;
const DWORD POLL_INTERVAL_MS = 250;
const NTSTATUS WAIT_TIMEOUT_STATUS = 0x102;

struct StartupDelay
{
    DWORD title_id;
    LONG generation;
    DWORD elapsed_ms;
    bool immediate;

    void Begin(DWORD titleId, LONG titleGeneration, bool running)
    {
        title_id = running ? titleId : 0;
        generation = titleGeneration;
        elapsed_ms = 0;
        immediate = running && titleId != 0;
    }

    bool Observe(DWORD titleId, LONG titleGeneration, bool running)
    {
        if (!running || !titleId)
        {
            title_id = 0;
            elapsed_ms = 0;
            immediate = false;
            return false;
        }
        if (title_id != titleId || generation != titleGeneration)
        {
            title_id = titleId;
            generation = titleGeneration;
            elapsed_ms = 0;
            immediate = false;
            return false;
        }
        if (immediate)
        {
            immediate = false;
            return true;
        }
        elapsed_ms += POLL_INTERVAL_MS;
        return elapsed_ms >= STARTUP_DELAY_MS;
    }
};

decltype(&XamGetCurrentTitleId) g_query_title;
decltype(&XamLoaderGetState) g_query_state;
decltype(&XNotifyQueueUI) g_queue_notification;
KEVENT g_stop_event;
PVOID g_startup_thread;
StartupDelay g_startup_delay;
volatile LONG g_stopping;
volatile LONG g_title_generation;
bool g_in_xenia;
WCHAR g_loaded_text[] = L"CoD Xe: Loaded";
WCHAR g_unloaded_text[] = L"CoD Xe: Unloaded";

bool IsTitleRunning()
{
    return g_query_state() == XamLoaderStateTitleRunning;
}

bool WaitForStop(DWORD milliseconds)
{
    LARGE_INTEGER timeout;
    timeout.QuadPart = -static_cast<LONGLONG>(milliseconds) * 10000;
    return KeWaitForSingleObject(&g_stop_event, Executive, KernelMode, FALSE, &timeout) != WAIT_TIMEOUT_STATUS;
}

struct Notification
{
    DWORD title_id;
    LONG generation;
    WCHAR *text;
    bool unloading;
    volatile LONG queued;
};

DWORD WINAPI QueueNotification(void *argument)
{
    Notification *notification = static_cast<Notification *>(argument);
    if (KeGetCurrentProcessType() == PROC_USER &&
        (notification->unloading || !InterlockedCompareExchange(&g_stopping, 0, 0)) &&
        notification->generation == InterlockedCompareExchange(&g_title_generation, 0, 0) &&
        g_query_title() == notification->title_id && IsTitleRunning())
    {
        g_queue_notification(XNOTIFYUI_TYPE_EXCLAIM, XUSER_INDEX_ANY, XNOTIFY_SYSTEM, notification->text, nullptr);
        InterlockedExchange(&notification->queued, 1);
    }
    return 0;
}

bool DispatchNotification(Notification &notification)
{
    // XNotifyQueueUI requires a title thread on hardware, even when the plugin is a system DLL.
    PVOID thread = nullptr;
    const NTSTATUS status = ExCreateThread(&thread, 0, nullptr, nullptr, QueueNotification, &notification,
                                           EX_CREATE_FLAG_TITLE_EXEC | EX_CREATE_FLAG_RETURN_KTHREAD);
    if (!NT_SUCCESS(status))
    {
        return false;
    }
    // XAM copies the text during the call. Keep the context and plugin alive until it returns.
    KeWaitForSingleObject(thread, Executive, KernelMode, FALSE, nullptr);
    ObDereferenceObject(thread);
    return InterlockedCompareExchange(&notification.queued, 0, 0) != 0;
}

DWORD WINAPI StartupThread(void *)
{
    StartupDelay delay = g_startup_delay;
    while (!WaitForStop(0))
    {
        const DWORD title_id = g_query_title();
        const LONG generation = InterlockedCompareExchange(&g_title_generation, 0, 0);
        if (delay.Observe(title_id, generation, title_id && IsTitleRunning()))
        {
            Notification notification = {title_id, generation, g_loaded_text, false, 0};
            if (DispatchNotification(notification))
            {
                DbgPrint("[codxe] Loaded.\n");
                return 0;
            }
            // A title transition may terminate the dispatcher before it submits the notification.
            delay.elapsed_ms = 0;
        }
        if (WaitForStop(POLL_INTERVAL_MS))
        {
            break;
        }
    }
    return 0;
}
} // namespace

void xbox::notifications::Initialize()
{
    HANDLE xam = nullptr;
    if (!NT_SUCCESS(XexGetModuleHandle("xam.xex", &xam)) ||
        !NT_SUCCESS(XexGetProcedureAddress(xam, 656, &g_queue_notification)) || !g_queue_notification)
    {
        return;
    }

    g_in_xenia = GetEnvironment() == ENVIRONMENT_XENIA;
    if (g_in_xenia)
    {
        // Xenia implements the popup itself and loads plugins directly into the game.
        g_queue_notification(XNOTIFYUI_TYPE_EXCLAIM, XUSER_INDEX_ANY, XNOTIFY_SYSTEM, g_loaded_text, nullptr);
        DbgPrint("[codxe] Loaded.\n");
        return;
    }

    if (!NT_SUCCESS(XexGetProcedureAddress(xam, 463, &g_query_title)) ||
        !NT_SUCCESS(XexGetProcedureAddress(xam, 1191, &g_query_state)) || !g_query_title || !g_query_state)
    {
        return;
    }

    // Resolve exports before the worker starts; it must not acquire the loader lock during detach.
    KeInitializeEvent(&g_stop_event, EventNotificationObject, FALSE);
    InterlockedExchange(&g_stopping, 0);
    const DWORD title_id = g_query_title();
    const LONG generation = InterlockedCompareExchange(&g_title_generation, 0, 0);
    // Snapshot at attach, before the worker runs: manual loading needs no boot delay.
    g_startup_delay.Begin(title_id, generation, title_id && IsTitleRunning());
    const NTSTATUS status = ExCreateThread(&g_startup_thread, 0, nullptr, nullptr, StartupThread, nullptr,
                                           EX_CREATE_FLAG_SYSTEM | EX_CREATE_FLAG_RETURN_KTHREAD);
    if (!NT_SUCCESS(status))
    {
        g_startup_thread = nullptr;
    }
}

void xbox::notifications::Shutdown()
{
    if (g_startup_thread)
    {
        InterlockedExchange(&g_stopping, 1);
        KeSetEvent(&g_stop_event, 0, FALSE);
        KeWaitForSingleObject(g_startup_thread, Executive, KernelMode, FALSE, nullptr);
        ObDereferenceObject(g_startup_thread);
        g_startup_thread = nullptr;
    }

    if (g_in_xenia || !g_queue_notification || !g_query_title || !g_query_state)
    {
        return;
    }
    const DWORD title_id = g_query_title();
    if (!title_id || !IsTitleRunning())
    {
        return;
    }
    Notification notification = {title_id, InterlockedCompareExchange(&g_title_generation, 0, 0), g_unloaded_text, true,
                                 0};
    DispatchNotification(notification);
}

void xbox::notifications::OnTitleTerminate()
{
    // A relaunch can keep the same title ID; invalidate any notification for the old instance.
    InterlockedIncrement(&g_title_generation);
}
