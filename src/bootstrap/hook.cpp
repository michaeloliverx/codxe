#include "hook.h"
#include <xkelib.h>
#include "../common/ppc.h"

namespace
{
#pragma section(".text")
__declspec(allocate(".text")) static DWORD trampoline[11];

void WriteBranch(volatile DWORD *out, DWORD target, bool linked)
{
    out[0] = 0x3D600000 | (target >> 16);    // lis r11
    out[1] = 0x616B0000 | (target & 0xFFFF); // ori r11,r11
    out[2] = 0x7D6903A6;                     // mtctr r11
    out[3] = 0x4E800420 | (linked ? 1 : 0);  // bctr/bctrl
}

void BuildLoaderTrampoline(const DWORD source[4], DWORD address)
{
    // Capture the caller's LR before the relocated call to __savegprlr_29 changes LR.
    // Replay the two remaining instructions, then resume after the overwritten prologue.
    trampoline[0] = source[0];
    WriteBranch(trampoline + 1, ppc::RelativeBranchTarget(address + 4, source[1]), true);
    trampoline[5] = source[2];
    trampoline[6] = source[3];
    WriteBranch(trampoline + 7, address + 4 * sizeof(DWORD), false);
}
} // namespace

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
    BuildLoaderTrampoline(saved, target.address);
    KeFlushCacheRange(trampoline, sizeof(trampoline));
    WriteBranch(source, reinterpret_cast<DWORD>(destination), false);
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
