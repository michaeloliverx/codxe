#pragma once

#include <xtl.h>
#include "xbox360.h"

namespace xbox
{
struct KernelProfile
{
    WORD build;
    Environment environment;
    DWORD finish_load_address;
    DWORD prologue[4];
};

const KernelProfile *FindKernelProfile(WORD build, Environment environment);
bool IsKernelHookValid(const KernelProfile &profile);
} // namespace xbox
