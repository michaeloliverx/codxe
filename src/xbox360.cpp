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

typedef int (*XNotifyQueueUI_t)(XNotifyQueueUIType type, DWORD user_index, DWORD areas, const WCHAR *display_text,
                                void *context_data);
static XNotifyQueueUI_t XNotifyQueueUI = reinterpret_cast<XNotifyQueueUI_t>(ResolveFunction("xam.xex", 656));

const DWORD XNOTIFY_USER_INDEX_ANY = 0x000000FF;
const DWORD XNOTIFY_AREA_SYSTEM = 0x00000001;

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

void ApplySystemPatches()
{
    // Allow XNotifyQueueUI to be called from system threads.
    *(volatile uint16_t *)0x816A3158 = 0x4800;
}

void Notify(const char *message)
{
    assert(message != nullptr);

    char branded_message[160];
    _snprintf_s(branded_message, sizeof(branded_message), _TRUNCATE, "CoD Xe: %s", message);

    WCHAR display_text[160];
    CopyAsciiToWide(branded_message, display_text, ARRAYSIZE(display_text));
    XNotifyQueueUI(XNOTIFYUI_TYPE_EXCLAIM, XNOTIFY_USER_INDEX_ANY, XNOTIFY_AREA_SYSTEM, display_text, nullptr);
}

} // namespace xbox
