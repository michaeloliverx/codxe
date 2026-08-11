#include "pch.h"
#include "ppc.h"

extern "C"
{
    NTSYSAPI VOID NTAPI KeFlushCacheRange(PVOID address, DWORD size);
}

namespace ppc
{
void Nop(uintptr_t address)
{
    *reinterpret_cast<volatile uint32_t *>(address) = 0x60000000;
    KeFlushCacheRange(reinterpret_cast<void *>(address), sizeof(uint32_t));
}
} // namespace ppc
