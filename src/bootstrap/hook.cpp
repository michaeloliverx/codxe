#include "hook.h"
#include <xkelib.h>
#include "ppc.h"

#pragma section(".text")
__declspec(allocate(".text")) static DWORD trampoline[11];

bool ResidentHook::Install(const xbox::KernelProfile &profile, const void *destination)
{
    if (!xbox::IsKernelHookValid(profile))
        return false;
    source = reinterpret_cast<DWORD *>(profile.finish_load_address);
    for (unsigned i = 0; i < 4; ++i)
        saved[i] = source[i];
    codxe::BuildLoaderTrampoline(saved, profile.finish_load_address, trampoline);
    KeFlushCacheRange(trampoline, sizeof(trampoline));
    codxe::WriteBranch(source, reinterpret_cast<DWORD>(destination), false);
    KeFlushCacheRange(source, sizeof(saved));
    return true;
}

void ResidentHook::Remove()
{
    for (unsigned i = 0; i < 4; ++i)
        static_cast<volatile DWORD *>(source)[i] = saved[i];
    KeFlushCacheRange(source, sizeof(saved));
    source = NULL;
}

void *ResidentHook::Original()
{
    return trampoline;
}
