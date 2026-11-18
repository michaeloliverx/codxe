#pragma once
#include <xtl.h>

namespace codxe
{
void WriteBranch(volatile DWORD *out, DWORD target, bool linked);
DWORD RelativeBranchTarget(DWORD address, DWORD instruction);
void BuildLoaderTrampoline(const DWORD source[4], DWORD address, DWORD out[11]);
} // namespace codxe
