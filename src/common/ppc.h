#pragma once

#include <stdint.h>

namespace ppc
{
void Nop(uintptr_t address);
void Branch(uintptr_t address, uintptr_t target);
void BranchLink(uintptr_t address, uintptr_t target);
} // namespace ppc
