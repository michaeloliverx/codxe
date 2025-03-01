#include <xtl.h>
#include "xboxkrnl.h"

namespace xbox
{
    XNOTIFYQUEUEUI XNotifyQueueUI = nullptr;

    void *ResolveFunction(const std::string &moduleName, uint32_t ordinal)
    {
        HMODULE moduleHandle = GetModuleHandle(moduleName.c_str());
        if (moduleHandle == nullptr)
            return nullptr;

        return GetProcAddress(moduleHandle, reinterpret_cast<const char *>(ordinal));
    }

    void show_notification(const wchar_t *message)
    {
        if (!XNotifyQueueUI) // Resolve XNotifyQueueUI if it hasn't been resolved yet
        {
            XNotifyQueueUI = static_cast<XNOTIFYQUEUEUI>(ResolveFunction("xam.xex", 656));
        }

        if (XNotifyQueueUI)
        {
            XNotifyQueueUI(0, 0, XNOTIFY_SYSTEM, message, nullptr);
        }
    }
}
