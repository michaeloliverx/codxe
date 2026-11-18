#pragma once

#include <xtl.h>

namespace xbox
{
struct KernelHookTarget
{
    DWORD address;
    DWORD prologue[4];
};

bool ResolveFinishLoad(KernelHookTarget &target);
} // namespace xbox
