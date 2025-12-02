#include "pch.h"

namespace xbox
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
bool IsXenia()
{
    // https://github.com/ClementDreptin/XexUtils/blob/ae8a8b832315678255c00d6a9b967a9136155503/src/Xam_.cpp#L168
    // https://github.com/RBEnhanced/RB3Enhanced/blob/106c4290ee4fc2a91da65f4092a2b881971dc59c/source/xbox360.c#L16

    // Cache the detection result so the lookup only happens once.
    static bool isXenia = (reinterpret_cast<uintptr_t>(ResolveFunction("xam.xex", 1)) >> 24) != 0x81;

    return isXenia;
}

} // namespace xbox
