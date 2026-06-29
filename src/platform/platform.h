#pragma once

namespace codxe
{

enum PlatformKind
{
    PLATFORM_UNKNOWN,
    PLATFORM_XENIA,
    PLATFORM_XBOX360_RETAIL,
    PLATFORM_XBOX360_DEVKIT,
};

struct PlatformInfo
{
    PlatformKind kind;
};

void PlatformInit(PlatformInfo *platform);
PlatformInfo DetectPlatform();
const char *GetPlatformName(PlatformKind kind);

} // namespace codxe
