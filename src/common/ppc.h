#pragma once

#include <stdint.h>

namespace ppc
{
inline uint32_t RelativeBranchTarget(uint32_t address, uint32_t instruction)
{
    // The 24-bit branch field plus two low zero bits is a signed byte offset from the instruction.
    uint32_t displacement = instruction & 0x03FFFFFC;
    if (displacement & 0x02000000)
        displacement |= 0xFC000000;
    return address + displacement;
}

void Nop(uintptr_t address);
void Branch(uintptr_t address, uintptr_t target);
void BranchLink(uintptr_t address, uintptr_t target);
} // namespace ppc
