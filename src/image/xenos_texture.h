#pragma once

#include <stddef.h>
#include <stdint.h>

namespace image
{
namespace xenos_texture
{
struct TextureFormatInfo
{
    uint32_t gpuFormat;
    uint32_t blockWidth;
    uint32_t blockHeight;
    uint32_t bytesPerBlock;
    uint32_t bitsPerPixel;
};

const TextureFormatInfo *GetTextureFormatInfo(uint32_t gpuFormat);
void ApplyGpuEndian(void *data, size_t size, GPUENDIAN endianType);
uint32_t GetTextureLevelCount(const D3DBaseTexture *texture);
uint32_t GetMipTailBaseLevel(uint32_t width, uint32_t height);
uint32_t CalculateLinearRowPitch(uint32_t width, uint32_t mipLevel, uint32_t gpuFormat);
uint32_t CalculateLinearLevelSize(uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t gpuFormat);
uint32_t CalculateTiledLevelSize(uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t gpuFormat,
                                 uint32_t basePitch);
uint32_t CalculateBaseSize(const D3DBaseTexture *texture, uint32_t width, uint32_t height, uint32_t faceCount);
uint32_t CalculateMipLevelOffset(uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t gpuFormat,
                                 uint32_t faceCount);
unsigned char *GetTextureBase(const D3DBaseTexture *texture, unsigned char *fallbackBaseData);
unsigned char *GetTextureMipBase(const D3DBaseTexture *texture, unsigned char *baseData, uint32_t width,
                                 uint32_t height, uint32_t gpuFormat, uint32_t faceCount);
bool TileTextureLevel(uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t gpuFormat, uint32_t basePitch,
                      void *destination, const void *source, uint32_t sourceRowPitch);
bool TileTextureLevel(uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t gpuFormat, uint32_t basePitch,
                      void *destination, size_t destinationSize, const void *source, size_t sourceSize,
                      uint32_t sourceRowPitch);
bool UntileTextureLevel(uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t gpuFormat, uint32_t basePitch,
                        void *destination, uint32_t destinationRowPitch, const void *source);
bool UntileTextureLevel(uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t gpuFormat, uint32_t basePitch,
                        void *destination, size_t destinationSize, uint32_t destinationRowPitch, const void *source,
                        size_t sourceSize);
} // namespace xenos_texture
} // namespace image
