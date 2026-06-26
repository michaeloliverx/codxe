#pragma once

#include <cstddef>
#include <cstdint>

namespace image
{
size_t CalculateRequiredLinearDataSize(uint32_t width, uint32_t height, GPUTEXTUREFORMAT format, uint32_t firstMipLevel,
                                       uint32_t levelCount, uint32_t faceCount);
uint32_t CalculateDdsMipOffset(uint32_t width, uint32_t height, GPUTEXTUREFORMAT format, uint32_t mipLevel);
size_t CalculateRequiredMipTextureBytes(uint32_t width, uint32_t height, GPUTEXTUREFORMAT format,
                                        uint32_t firstMipLevel, uint32_t levelCount, uint32_t faceCount);
} // namespace image
