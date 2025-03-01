#include <xtl.h>
#include <cstdint>
#include <string>

namespace xbox
{
    extern "C"
    {
        void DbgPrint(const char *format, ...);

        uint32_t ExCreateThread(
            HANDLE *pHandle,
            uint32_t stackSize,
            uint32_t *pThreadId,
            void *pApiThreadStartup,
            PTHREAD_START_ROUTINE pStartAddress,
            void *pParameter,
            uint32_t creationFlags);

        uint32_t XamGetCurrentTitleId();
    }

    // Function pointer type for XNotifyQueueUI
    typedef void (*XNOTIFYQUEUEUI)(uint32_t type, uint32_t userIndex, uint64_t areas, const wchar_t *displayText, void *pContextData);

    // Function to display Xbox notifications
    void show_notification(const wchar_t *message);
}
