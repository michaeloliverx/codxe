#include "pch.h"
#include "image/texture_layout.h"
#include "image/xenos_texture.h"

namespace image
{
size_t CalculateRequiredLinearDataSize(uint32_t width, uint32_t height, GPUTEXTUREFORMAT format, uint32_t firstMipLevel,
                                       uint32_t levelCount, uint32_t faceCount)
{
    size_t requiredSize = 0;

    for (uint32_t localMipLevel = 0; localMipLevel < levelCount; ++localMipLevel)
    {
        const uint32_t mipLevel = firstMipLevel + localMipLevel;
        const uint32_t levelSize = xenos_texture::CalculateLinearLevelSize(width, height, mipLevel, format);
        if (levelSize == 0)
            return 0;

        requiredSize += static_cast<size_t>(levelSize) * faceCount;
    }

    return requiredSize;
}

uint32_t CalculateDdsMipOffset(uint32_t width, uint32_t height, GPUTEXTUREFORMAT format, uint32_t mipLevel)
{
    uint32_t offset = 0;

    for (uint32_t currentMip = 0; currentMip < mipLevel; ++currentMip)
    {
        const uint32_t levelSize = xenos_texture::CalculateLinearLevelSize(width, height, currentMip, format);
        if (levelSize == 0)
            return 0;

        offset += levelSize;
    }

    return offset;
}

size_t CalculateRequiredMipTextureBytes(uint32_t width, uint32_t height, GPUTEXTUREFORMAT format,
                                        uint32_t firstMipLevel, uint32_t levelCount, uint32_t faceCount)
{
    size_t requiredSize = 0;

    for (uint32_t mipLevel = firstMipLevel; mipLevel < levelCount; ++mipLevel)
    {
        const uint32_t levelSize = xenos_texture::CalculateTiledLevelSize(width, height, mipLevel, format, 0u);
        if (levelSize == 0)
            return 0;

        requiredSize += static_cast<size_t>(levelSize) * faceCount;
    }

    return requiredSize;
}
} // namespace image
