#include "pch.h"

namespace xbox
{
namespace
{

void *ResolveFunction(const char *moduleName, unsigned int ordinal)
{
    HMODULE moduleHandle = GetModuleHandle(moduleName);
    if (moduleHandle == nullptr)
        return nullptr;

    return GetProcAddress(moduleHandle, reinterpret_cast<const char *>(ordinal));
}

/**
 * Check if we are running in Xenia Canary.
 *
 * https://github.com/xenia-canary/xenia-canary
 */
bool DetectXenia()
{
    // https://github.com/ClementDreptin/XexUtils/blob/ae8a8b832315678255c00d6a9b967a9136155503/src/Xam_.cpp#L168
    // https://github.com/RBEnhanced/RB3Enhanced/blob/106c4290ee4fc2a91da65f4092a2b881971dc59c/source/xbox360.c#L16

    return (reinterpret_cast<uintptr_t>(ResolveFunction("xam.xex", 1)) >> 24) != 0x81;
}

bool DetectDevkit()
{
    return (*(DWORD *)0x8E038610 & 0x8000) ? false : true;
}

Environment DetectEnvironment()
{
    if (DetectXenia())
    {
        return ENVIRONMENT_XENIA;
    }

    return DetectDevkit() ? ENVIRONMENT_XBOX_DEVKIT : ENVIRONMENT_XBOX_RETAIL;
}
} // namespace

Environment GetEnvironment()
{
    static Environment environment = DetectEnvironment();
    return environment;
}

const char *GetEnvironmentName(Environment environment)
{
    switch (environment)
    {
    case ENVIRONMENT_XENIA:
        return "Xenia";
    case ENVIRONMENT_XBOX_DEVKIT:
        return "Xbox 360 Devkit";
    case ENVIRONMENT_XBOX_RETAIL:
        return "Xbox 360";
    default:
        assert(false);
        return "Unknown";
    }
}

} // namespace xbox
