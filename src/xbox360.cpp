#include "pch.h"

namespace xbox
{
namespace
{

void *ResolveFunction(const char *moduleName, unsigned int ordinal)
{
    HMODULE moduleHandle = GetModuleHandle(moduleName);
    if (moduleHandle == nullptr)
        return nullptr;

    return GetProcAddress(moduleHandle, reinterpret_cast<const char *>(ordinal));
}

enum XNotifyQueueUIType
{
    XNOTIFYUI_TYPE_GENERIC = 3,
    XNOTIFYUI_TYPE_EXCLAIM = 34,
};

typedef void (*XNotifyQueueUI_t)(XNotifyQueueUIType type, DWORD user_index, uint64_t areas, const WCHAR *display_text,
                                 void *context_data);
static XNotifyQueueUI_t XNotifyQueueUI = reinterpret_cast<XNotifyQueueUI_t>(ResolveFunction("xam.xex", 656));

const DWORD XNOTIFY_USER_INDEX_ANY = 0x000000FF;
const uint64_t XNOTIFY_AREA_SYSTEM = 0x0000000000000001;
const DWORD XNOTIFY_DELAY_MS = 1000;

struct Notification
{
    volatile LONG inUse;
    WCHAR displayText[160];
};

Notification g_notifications[2];

/**
 * Check if we are running in Xenia Canary.
 *
 * https://github.com/xenia-canary/xenia-canary
 */
bool DetectXenia()
{
    // https://github.com/ClementDreptin/XexUtils/blob/ae8a8b832315678255c00d6a9b967a9136155503/src/Xam_.cpp#L168
    // https://github.com/RBEnhanced/RB3Enhanced/blob/106c4290ee4fc2a91da65f4092a2b881971dc59c/source/xbox360.c#L16

    return (reinterpret_cast<uintptr_t>(ResolveFunction("xam.xex", 1)) >> 24) != 0x81;
}

bool DetectDevkit()
{
    return (*(DWORD *)0x8E038610 & 0x8000) ? false : true;
}

Environment DetectEnvironment()
{
    if (DetectXenia())
    {
        return ENVIRONMENT_XENIA;
    }

    return DetectDevkit() ? ENVIRONMENT_XBOX_DEVKIT : ENVIRONMENT_XBOX_RETAIL;
}

void CopyAsciiToWide(const char *source, WCHAR *destination, size_t destination_count)
{
    if (destination == nullptr || destination_count == 0)
    {
        return;
    }

    size_t i = 0;
    if (source != nullptr)
    {
        for (; source[i] != '\0' && i + 1 < destination_count; i++)
        {
            destination[i] = static_cast<unsigned char>(source[i]);
        }
    }

    destination[i] = L'\0';
}

DWORD WINAPI NotifyThread(void *argument)
{
    Notification *notification = static_cast<Notification *>(argument);

    Sleep(XNOTIFY_DELAY_MS);
    XNotifyQueueUI(XNOTIFYUI_TYPE_EXCLAIM, XNOTIFY_USER_INDEX_ANY, XNOTIFY_AREA_SYSTEM, notification->displayText,
                   nullptr);
    InterlockedExchange(&notification->inUse, 0);
    return 0;
}
} // namespace

Environment GetEnvironment()
{
    static Environment environment = DetectEnvironment();
    return environment;
}

const char *GetEnvironmentName(Environment environment)
{
    switch (environment)
    {
    case ENVIRONMENT_XENIA:
        return "Xenia";
    case ENVIRONMENT_XBOX_DEVKIT:
        return "Xbox 360 Devkit";
    case ENVIRONMENT_XBOX_RETAIL:
        return "Xbox 360";
    default:
        assert(false);
        return "Unknown";
    }
}

void Notify(const char *message)
{
    assert(message != nullptr);

    Notification *notification = nullptr;
    for (size_t i = 0; i < ARRAYSIZE(g_notifications); i++)
    {
        if (InterlockedCompareExchange(&g_notifications[i].inUse, 1, 0) == 0)
        {
            notification = &g_notifications[i];
            break;
        }
    }

    if (notification == nullptr)
    {
        DbgPrint("[codxe] Notification queue is full.\n");
        return;
    }

    char branded_message[160];
    _snprintf_s(branded_message, sizeof(branded_message), _TRUNCATE, "CoD Xe: %s", message);
    CopyAsciiToWide(branded_message, notification->displayText, ARRAYSIZE(notification->displayText));

    HANDLE thread = nullptr;
    const NTSTATUS status =
        ExCreateThread(&thread, 0, nullptr, nullptr, NotifyThread, notification, EX_CREATE_FLAG_TITLE_EXEC);
    if (!NT_SUCCESS(status))
    {
        InterlockedExchange(&notification->inUse, 0);
        DbgPrint("[codxe] Failed to create notification thread: 0x%08X.\n", status);
        return;
    }

    CloseHandle(thread);
}

} // namespace xbox
