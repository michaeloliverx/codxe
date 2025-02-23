#include <xtl.h>
#include <cstdint>

// Declarations for Xbox 360 kernel functions not exposed in the official SDK.
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
