#pragma once

#include "config.h"
#include "version.h"
namespace branding
{
inline const char *GetBrandingString()
{
    static char brandingBuffer[256];

    if (Config::GetActiveMod()[0] == '\0')
    {
        _snprintf_s(brandingBuffer, sizeof(brandingBuffer), "CoD ^2Xe^7 r%d", BUILD_NUMBER);
    }
    else
    {
        _snprintf_s(brandingBuffer, sizeof(brandingBuffer), "CoD ^2Xe^7 r%d\nMod: %s", BUILD_NUMBER,
                    Config::GetActiveMod());
    }

    return brandingBuffer;
}
} // namespace branding
