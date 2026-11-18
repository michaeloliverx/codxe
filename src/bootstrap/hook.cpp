#include "hook.h"
#include <xkelib.h>
#include "ppc.h"

#pragma section(".text")
__declspec(allocate(".text")) static DWORD trampoline[11];

bool ResidentHook::Install(const xbox::KernelHookTarget &target, const void *destination)
{
    DWORD *instructions = reinterpret_cast<DWORD *>(target.address);
    for (unsigned i = 0; i < ARRAYSIZE(saved); ++i)
    {
        if (!MmIsAddressValid(instructions + i) ||
            static_cast<const volatile DWORD *>(instructions)[i] != target.prologue[i])
        {
            DbgPrint("[codxe][loader] Kernel hook mismatch at %p.\n", instructions + i);
            return false;
        }
        saved[i] = target.prologue[i];
    }
    source = instructions;
    codxe::BuildLoaderTrampoline(saved, target.address, trampoline);
    KeFlushCacheRange(trampoline, sizeof(trampoline));
    codxe::WriteBranch(source, reinterpret_cast<DWORD>(destination), false);
    KeFlushCacheRange(source, sizeof(saved));
    return true;
}

void ResidentHook::Remove()
{
    for (unsigned i = 0; i < ARRAYSIZE(saved); ++i)
        static_cast<volatile DWORD *>(source)[i] = saved[i];
    KeFlushCacheRange(source, sizeof(saved));
    source = NULL;
}

void *ResidentHook::Original()
{
    return trampoline;
}
