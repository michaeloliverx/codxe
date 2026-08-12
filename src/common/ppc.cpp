#include "pch.h"
#include "ppc.h"

extern "C"
{
    NTSYSAPI VOID NTAPI KeFlushCacheRange(PVOID address, DWORD size);
}

namespace
{
void WriteInstruction(uintptr_t address, uint32_t instruction)
{
    *reinterpret_cast<volatile uint32_t *>(address) = instruction;
    KeFlushCacheRange(reinterpret_cast<void *>(address), sizeof(instruction));
}

uint32_t EncodeBranch(uintptr_t address, uintptr_t target)
{
    const int64_t displacement = static_cast<int64_t>(target) - static_cast<int64_t>(address);

    assert((displacement & 3) == 0);
    assert(displacement >= -0x02000000 && displacement <= 0x01FFFFFC);

    return 0x48000000 | (static_cast<uint32_t>(displacement) & 0x03FFFFFC);
}
} // namespace

namespace ppc
{
void Nop(uintptr_t address)
{
    WriteInstruction(address, 0x60000000);
}

void Branch(uintptr_t address, uintptr_t target)
{
    WriteInstruction(address, EncodeBranch(address, target));
}

void BranchLink(uintptr_t address, uintptr_t target)
{
    WriteInstruction(address, EncodeBranch(address, target) | 1u);
}
} // namespace ppc
