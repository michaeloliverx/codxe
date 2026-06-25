#include "pch.h"
#include "xenos_texture.h"

namespace
{
struct TextureLevelLayout
{
    uint32_t widthBlocks;
    uint32_t heightBlocks;
    uint32_t rowPitchBytes;
    uint32_t storedWidthBlocks;
    uint32_t storedHeightBlocks;
    uint32_t arraySliceStrideBytes;
};

uint32_t DivideRoundUp(uint32_t value, uint32_t divisor)
{
    return (value + divisor - 1) / divisor;
}

uint32_t AlignTo(uint32_t value, uint32_t alignment)
{
    return DivideRoundUp(value, alignment) * alignment;
}

uint32_t NextPow2(uint32_t value)
{
    if (value <= 1)
        return 1;

    uint32_t result = 1;
    while (result < value)
        result <<= 1;
    return result;
}

uint32_t Log2Ceil(uint32_t value)
{
    uint32_t result = 0;
    uint32_t current = value - 1;
    while (current != 0)
    {
        current >>= 1;
        ++result;
    }
    return result;
}

uint32_t CalculatePitch(uint32_t width, const xenos_texture::TextureFormatInfo &formatInfo)
{
    if (formatInfo.blockWidth > 1)
    {
        const uint32_t widthInBlocks = max(1u, DivideRoundUp(width, formatInfo.blockWidth));
        return AlignTo(widthInBlocks, 32u) / 8u;
    }

    return AlignTo(width, 32u) / 32u;
}

TextureLevelLayout CalculateLevelLayout(uint32_t width, uint32_t height, uint32_t mipLevel,
                                        const xenos_texture::TextureFormatInfo &formatInfo, uint32_t basePitch)
{
    const uint32_t mipWidth = max(width >> mipLevel, 1u);
    const uint32_t mipHeight = max(height >> mipLevel, 1u);

    TextureLevelLayout layout = {};
    layout.widthBlocks = max(1u, DivideRoundUp(mipWidth, formatInfo.blockWidth));
    layout.heightBlocks = max(1u, DivideRoundUp(mipHeight, formatInfo.blockHeight));

    if (mipLevel == 0)
    {
        const uint32_t pitch = basePitch != 0 ? basePitch : CalculatePitch(width, formatInfo);
        const uint32_t rowPitchTexels = pitch << 5u;
        layout.rowPitchBytes = max(1u, DivideRoundUp(rowPitchTexels, formatInfo.blockWidth)) * formatInfo.bytesPerBlock;
        layout.storedWidthBlocks = layout.rowPitchBytes / formatInfo.bytesPerBlock;
        layout.storedHeightBlocks = AlignTo(layout.heightBlocks, 32u);
    }
    else
    {
        const uint32_t mipWidthTexels = max(NextPow2(width) >> mipLevel, 1u);
        const uint32_t mipHeightTexels = max(NextPow2(height) >> mipLevel, 1u);
        layout.storedWidthBlocks = AlignTo(DivideRoundUp(mipWidthTexels, formatInfo.blockWidth), 32u);
        layout.storedHeightBlocks = AlignTo(DivideRoundUp(mipHeightTexels, formatInfo.blockHeight), 32u);
        layout.rowPitchBytes = layout.storedWidthBlocks * formatInfo.bytesPerBlock;
    }

    layout.arraySliceStrideBytes = AlignTo(layout.rowPitchBytes * layout.storedHeightBlocks, 4096u);
    return layout;
}

uint32_t CalculateLog2BytesPerBlock(uint32_t bytesPerBlock)
{
    return (bytesPerBlock / 4u) + ((bytesPerBlock / 2u) >> (bytesPerBlock / 4u));
}

uint32_t TiledOffset2DRow(uint32_t y, uint32_t width, uint32_t log2BytesPerBlock)
{
    const uint32_t macro = ((y / 32u) * (width / 32u)) << (log2BytesPerBlock + 7u);
    const uint32_t micro = ((y & 6u) << 2u) << log2BytesPerBlock;
    return macro + ((micro & ~0xFu) << 1u) + (micro & 0xFu) + ((y & 8u) << (3u + log2BytesPerBlock)) + ((y & 1u) << 4u);
}

uint32_t TiledOffset2DColumn(uint32_t x, uint32_t y, uint32_t log2BytesPerBlock, uint32_t baseOffset)
{
    const uint32_t macro = (x / 32u) << (log2BytesPerBlock + 7u);
    const uint32_t micro = (x & 7u) << log2BytesPerBlock;
    const uint32_t offset = baseOffset + macro + ((micro & ~0xFu) << 1u) + (micro & 0xFu);
    return ((offset & ~0x1FFu) << 3u) + ((offset & 0x1C0u) << 2u) + (offset & 0x3Fu) + ((y & 16u) << 7u) +
           (((((y & 8u) >> 2u) + (x >> 3u)) & 3u) << 6u);
}

void EndianSwap8In16(unsigned char *data, size_t size)
{
    for (size_t i = 0; i + 1 < size; i += 2)
        std::swap(data[i], data[i + 1]);
}

void EndianSwap8In32(unsigned char *data, size_t size)
{
    for (size_t i = 0; i + 3 < size; i += 4)
    {
        std::swap(data[i], data[i + 3]);
        std::swap(data[i + 1], data[i + 2]);
    }
}

void EndianSwap16In32(unsigned char *data, size_t size)
{
    for (size_t i = 0; i + 3 < size; i += 4)
    {
        std::swap(data[i], data[i + 2]);
        std::swap(data[i + 1], data[i + 3]);
    }
}
} // namespace

namespace xenos_texture
{
const TextureFormatInfo *GetTextureFormatInfo(uint32_t gpuFormat)
{
    static const TextureFormatInfo formats[] = {
        {GPUTEXTUREFORMAT_8, 1u, 1u, 1u, 8u},        {GPUTEXTUREFORMAT_8_8, 1u, 1u, 2u, 16u},
        {GPUTEXTUREFORMAT_8_8_8_8, 1u, 1u, 4u, 32u}, {GPUTEXTUREFORMAT_DXT1, 4u, 4u, 8u, 4u},
        {GPUTEXTUREFORMAT_DXT2_3, 4u, 4u, 16u, 8u},  {GPUTEXTUREFORMAT_DXT4_5, 4u, 4u, 16u, 8u},
        {GPUTEXTUREFORMAT_DXN, 4u, 4u, 16u, 8u},     {GPUTEXTUREFORMAT_DXT3A, 4u, 4u, 8u, 4u},
        {GPUTEXTUREFORMAT_DXT5A, 4u, 4u, 8u, 4u},
    };

    for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); ++i)
    {
        if (formats[i].gpuFormat == gpuFormat)
            return &formats[i];
    }

    return NULL;
}

void ApplyGpuEndian(void *data, size_t size, GPUENDIAN endianType)
{
    unsigned char *bytes = static_cast<unsigned char *>(data);
    switch (endianType)
    {
    case GPUENDIAN_8IN16:
        EndianSwap8In16(bytes, size);
        break;
    case GPUENDIAN_8IN32:
        EndianSwap8In32(bytes, size);
        break;
    case GPUENDIAN_16IN32:
        EndianSwap16In32(bytes, size);
        break;
    default:
        break;
    }
}

uint32_t GetTextureLevelCount(const D3DBaseTexture *texture)
{
    if (texture == NULL)
        return 1;
    return max(1u, static_cast<uint32_t>(texture->Format.MaxMipLevel) + 1u);
}

uint32_t GetMipTailBaseLevel(uint32_t width, uint32_t height)
{
    const uint32_t log2Size = Log2Ceil(min(width, height));
    return log2Size > 4u ? log2Size - 4u : 0u;
}

uint32_t CalculateLinearRowPitch(uint32_t width, uint32_t mipLevel, uint32_t gpuFormat)
{
    const TextureFormatInfo *formatInfo = GetTextureFormatInfo(gpuFormat);
    if (formatInfo == NULL)
        return 0;

    const uint32_t mipWidth = max(width >> mipLevel, 1u);
    const uint32_t widthBlocks = max(1u, DivideRoundUp(mipWidth, formatInfo->blockWidth));
    return widthBlocks * formatInfo->bytesPerBlock;
}

uint32_t CalculateLinearLevelSize(uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t gpuFormat)
{
    const TextureFormatInfo *formatInfo = GetTextureFormatInfo(gpuFormat);
    if (formatInfo == NULL)
        return 0;

    const uint32_t mipWidth = max(width >> mipLevel, 1u);
    const uint32_t mipHeight = max(height >> mipLevel, 1u);
    const uint32_t widthBlocks = max(1u, DivideRoundUp(mipWidth, formatInfo->blockWidth));
    const uint32_t heightBlocks = max(1u, DivideRoundUp(mipHeight, formatInfo->blockHeight));
    return widthBlocks * heightBlocks * formatInfo->bytesPerBlock;
}

uint32_t CalculateTiledLevelSize(uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t gpuFormat,
                                 uint32_t basePitch)
{
    const TextureFormatInfo *formatInfo = GetTextureFormatInfo(gpuFormat);
    if (formatInfo == NULL)
        return 0;

    const TextureLevelLayout layout = CalculateLevelLayout(width, height, mipLevel, *formatInfo, basePitch);
    return layout.arraySliceStrideBytes;
}

uint32_t CalculateBaseSize(const D3DBaseTexture *texture, uint32_t width, uint32_t height, uint32_t faceCount)
{
    if (texture == NULL)
        return 0;

    const TextureFormatInfo *formatInfo = GetTextureFormatInfo(texture->Format.DataFormat);
    if (formatInfo == NULL)
        return 0;

    const TextureLevelLayout layout = CalculateLevelLayout(width, height, 0u, *formatInfo, texture->Format.Pitch);
    return layout.arraySliceStrideBytes * faceCount;
}

uint32_t CalculateMipLevelOffset(uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t gpuFormat,
                                 uint32_t faceCount)
{
    const TextureFormatInfo *formatInfo = GetTextureFormatInfo(gpuFormat);
    if (formatInfo == NULL || mipLevel <= 1u)
        return 0;

    uint32_t offset = 0;
    for (uint32_t level = 1u; level < mipLevel; ++level)
    {
        const TextureLevelLayout layout = CalculateLevelLayout(width, height, level, *formatInfo, 0u);
        offset += layout.arraySliceStrideBytes * faceCount;
    }

    return offset;
}

unsigned char *GetTextureBase(const D3DBaseTexture *texture, unsigned char *fallbackBaseData)
{
    if (fallbackBaseData != NULL)
        return fallbackBaseData;

    if (texture != NULL && texture->Format.BaseAddress != 0)
        return reinterpret_cast<unsigned char *>(texture->Format.BaseAddress << 12u);

    return NULL;
}

unsigned char *GetTextureMipBase(const D3DBaseTexture *texture, unsigned char *baseData, uint32_t width,
                                 uint32_t height, uint32_t gpuFormat, uint32_t faceCount)
{
    const uint32_t baseSize = CalculateBaseSize(texture, width, height, faceCount);
    if (baseData != NULL)
        return baseData + baseSize;

    if (texture != NULL && texture->Format.MipAddress != 0)
        return reinterpret_cast<unsigned char *>(texture->Format.MipAddress << 12u);

    return NULL;
}

bool TileTextureLevel(uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t gpuFormat, uint32_t basePitch,
                      void *destination, const void *source, uint32_t sourceRowPitch)
{
    return TileTextureLevel(width, height, mipLevel, gpuFormat, basePitch, destination, static_cast<size_t>(-1), source,
                            static_cast<size_t>(-1), sourceRowPitch);
}

bool TileTextureLevel(uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t gpuFormat, uint32_t basePitch,
                      void *destination, size_t destinationSize, const void *source, size_t sourceSize,
                      uint32_t sourceRowPitch)
{
    const TextureFormatInfo *formatInfo = GetTextureFormatInfo(gpuFormat);
    if (formatInfo == NULL || destination == NULL || source == NULL || sourceRowPitch == 0)
    {
        return false;
    }

    const TextureLevelLayout layout = CalculateLevelLayout(width, height, mipLevel, *formatInfo, basePitch);
    if (sourceRowPitch < layout.widthBlocks * formatInfo->bytesPerBlock)
    {
        return false;
    }

    const uint32_t log2BytesPerBlock = CalculateLog2BytesPerBlock(formatInfo->bytesPerBlock);
    const unsigned char *sourceBytes = static_cast<const unsigned char *>(source);
    unsigned char *destinationBytes = static_cast<unsigned char *>(destination);

    for (uint32_t y = 0; y < layout.heightBlocks; ++y)
    {
        const uint32_t destinationRowOffset = TiledOffset2DRow(y, layout.storedWidthBlocks, log2BytesPerBlock);

        for (uint32_t x = 0; x < layout.widthBlocks; ++x)
        {
            uint32_t tiledOffset = TiledOffset2DColumn(x, y, log2BytesPerBlock, destinationRowOffset);
            tiledOffset >>= log2BytesPerBlock;

            const size_t sourceOffset =
                static_cast<size_t>(y) * sourceRowPitch + static_cast<size_t>(x) * formatInfo->bytesPerBlock;
            const size_t destinationOffset = static_cast<size_t>(tiledOffset) * formatInfo->bytesPerBlock;
            if (sourceOffset + formatInfo->bytesPerBlock > sourceSize ||
                destinationOffset + formatInfo->bytesPerBlock > destinationSize)
            {
                return false;
            }

            memcpy(destinationBytes + destinationOffset, sourceBytes + sourceOffset, formatInfo->bytesPerBlock);
        }
    }

    return true;
}

bool UntileTextureLevel(uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t gpuFormat, uint32_t basePitch,
                        void *destination, uint32_t destinationRowPitch, const void *source)
{
    return UntileTextureLevel(width, height, mipLevel, gpuFormat, basePitch, destination, static_cast<size_t>(-1),
                              destinationRowPitch, source, static_cast<size_t>(-1));
}

bool UntileTextureLevel(uint32_t width, uint32_t height, uint32_t mipLevel, uint32_t gpuFormat, uint32_t basePitch,
                        void *destination, size_t destinationSize, uint32_t destinationRowPitch, const void *source,
                        size_t sourceSize)
{
    const TextureFormatInfo *formatInfo = GetTextureFormatInfo(gpuFormat);
    if (formatInfo == NULL || destination == NULL || source == NULL || destinationRowPitch == 0)
        return false;

    const TextureLevelLayout layout = CalculateLevelLayout(width, height, mipLevel, *formatInfo, basePitch);
    if (destinationRowPitch < layout.widthBlocks * formatInfo->bytesPerBlock)
        return false;

    const uint32_t log2BytesPerBlock = CalculateLog2BytesPerBlock(formatInfo->bytesPerBlock);
    unsigned char *destinationBytes = static_cast<unsigned char *>(destination);
    const unsigned char *sourceBytes = static_cast<const unsigned char *>(source);

    for (uint32_t y = 0; y < layout.heightBlocks; ++y)
    {
        const uint32_t sourceRowOffset = TiledOffset2DRow(y, layout.storedWidthBlocks, log2BytesPerBlock);
        for (uint32_t x = 0; x < layout.widthBlocks; ++x)
        {
            uint32_t tiledOffset = TiledOffset2DColumn(x, y, log2BytesPerBlock, sourceRowOffset);
            tiledOffset >>= log2BytesPerBlock;

            const size_t destinationOffset =
                static_cast<size_t>(y) * destinationRowPitch + static_cast<size_t>(x) * formatInfo->bytesPerBlock;
            const size_t sourceOffset = static_cast<size_t>(tiledOffset) * formatInfo->bytesPerBlock;
            if (destinationOffset + formatInfo->bytesPerBlock > destinationSize ||
                sourceOffset + formatInfo->bytesPerBlock > sourceSize)
                return false;

            memcpy(destinationBytes + destinationOffset, sourceBytes + sourceOffset, formatInfo->bytesPerBlock);
        }
    }

    return true;
}
} // namespace xenos_texture
