#include "common.h"

namespace xbox
{
    XNOTIFYQUEUEUI XNotifyQueueUI = nullptr;

    bool InXenia()
    {
        // https://github.com/ClementDreptin/XexUtils/blob/ae8a8b832315678255c00d6a9b967a9136155503/src/Xam_.cpp#L168
        // https://github.com/RBEnhanced/RB3Enhanced/blob/106c4290ee4fc2a91da65f4092a2b881971dc59c/source/xbox360.c#L16

        void *xamFirstExport = ResolveFunction("xam.xex", 1);
        // If Xam is not in the typical memory address space, we're in an emulator
        return reinterpret_cast<uintptr_t>(xamFirstExport) >> 24 != 0x81;
    }

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
