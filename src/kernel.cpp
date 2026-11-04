#include "pch.h"
#include "kernel.h"

namespace
{
const xbox::KernelProfile KERNEL_PROFILES[] = {
    {6717, xbox::ENVIRONMENT_XBOX_RETAIL, 0x80075EA8, {0x7D8802A6, 0x48077941, 0x9421FF90, 0x7C7E1B78}},
    {16547, xbox::ENVIRONMENT_XBOX_RETAIL, 0x8007A798, {0x7D8802A6, 0x48093241, 0x9421FF90, 0x7C7E1B78}},
    {17559, xbox::ENVIRONMENT_XBOX_RETAIL, 0x8007AF68, {0x7D8802A6, 0x48091C61, 0x9421FF90, 0x7C7E1B78}},
    {17489, xbox::ENVIRONMENT_XBOX_DEVKIT, 0x800A17C8, {0x7D8802A6, 0x480ABBE1, 0x9421FF90, 0x7C7E1B78}},
};
}

const xbox::KernelProfile *xbox::FindKernelProfile(WORD build, Environment environment)
{
    // RGLoader reports 17559 while using the 17489 development binaries.
    if (environment == ENVIRONMENT_XBOX_DEVKIT && build == 17559)
    {
        build = 17489;
    }

    for (size_t i = 0; i < ARRAYSIZE(KERNEL_PROFILES); ++i)
    {
        const KernelProfile &profile = KERNEL_PROFILES[i];
        if (profile.build == build && profile.environment == environment)
        {
            return &profile;
        }
    }
    return nullptr;
}

bool xbox::IsKernelHookValid(const KernelProfile &profile)
{
    const DWORD *instructions = reinterpret_cast<const DWORD *>(profile.finish_load_address);
    for (size_t i = 0; i < ARRAYSIZE(profile.prologue); ++i)
    {
        const DWORD address = profile.finish_load_address + static_cast<DWORD>(i * sizeof(DWORD));
        if (!MmIsAddressValid(const_cast<DWORD *>(instructions + i)))
        {
            DbgPrint("[codxe] Kernel hook rejected: unreadable address 0x%08X.\n", address);
            return false;
        }
        const DWORD instruction = instructions[i];
        if (instruction != profile.prologue[i])
        {
            DbgPrint("[codxe] Kernel hook rejected at 0x%08X: expected 0x%08X, found 0x%08X.\n", address,
                     profile.prologue[i], instruction);
            return false;
        }
    }
    return true;
}
