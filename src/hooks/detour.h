#pragma once

#include <xtl.h>

namespace codxe
{

const size_t kDetourOriginalInstructionCapacity = 32;

struct Detour
{
    void *source;
    const void *target;
    BYTE *trampoline;
    BYTE original_instructions[kDetourOriginalInstructionCapacity];
    SIZE_T original_length;
};

void DetourInit(Detour *detour, void *source, const void *target);
bool DetourInstall(Detour *detour);
bool DetourRemove(Detour *detour);
SIZE_T DetourGetTrampolinePoolSize();
void DetourResetTrampolinePool(SIZE_T size);

template <typename T> T DetourGetOriginal(const Detour *detour)
{
    return detour != 0 ? reinterpret_cast<T>(detour->trampoline) : 0;
}

} // namespace codxe
