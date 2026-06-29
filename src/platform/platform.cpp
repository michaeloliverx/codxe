#include "platform/platform.h"

#include "platform/xbox360.h"

namespace codxe
{
namespace
{

bool DetectXenia()
{
    void *xam_export = ResolveExport("xam.xex", 1);
    return ((reinterpret_cast<uintptr_t>(xam_export) >> 24) != 0x81);
}

bool DetectDevkit()
{
    return (*(volatile DWORD *)0x8E038610 & 0x8000) == 0;
}

} // namespace

void PlatformInit(PlatformInfo *platform)
{
    if (platform == 0)
        return;

    platform->kind = PLATFORM_UNKNOWN;
}

PlatformInfo DetectPlatform()
{
    PlatformInfo platform = {};

    if (DetectXenia())
    {
        platform.kind = PLATFORM_XENIA;
        return platform;
    }

    platform.kind = DetectDevkit() ? PLATFORM_XBOX360_DEVKIT : PLATFORM_XBOX360_RETAIL;
    return platform;
}

const char *GetPlatformName(PlatformKind kind)
{
    switch (kind)
    {
    case PLATFORM_XENIA:
        return "Xenia";
    case PLATFORM_XBOX360_RETAIL:
        return "Xbox 360 retail";
    case PLATFORM_XBOX360_DEVKIT:
        return "Xbox 360 devkit";
    default:
        return "unknown";
    }
}

} // namespace codxe
