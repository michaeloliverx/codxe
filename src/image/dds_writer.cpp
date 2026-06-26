#include "pch.h"
#include "image/dds_writer.h"
#include "utils/endian.h"

namespace image
{
namespace
{
void ByteSwapDDSPixelFormat(DDS_PIXELFORMAT &pixelFormat)
{
    utils::endian::ByteSwap(pixelFormat.dwSize);
    utils::endian::ByteSwap(pixelFormat.dwFlags);
    utils::endian::ByteSwap(pixelFormat.dwFourCC);
    utils::endian::ByteSwap(pixelFormat.dwRGBBitCount);
    utils::endian::ByteSwap(pixelFormat.dwRBitMask);
    utils::endian::ByteSwap(pixelFormat.dwGBitMask);
    utils::endian::ByteSwap(pixelFormat.dwBBitMask);
    utils::endian::ByteSwap(pixelFormat.dwABitMask);
}

void ByteSwapDDSHeader(DDS_HEADER &header)
{
    utils::endian::ByteSwap(header.dwSize);
    utils::endian::ByteSwap(header.dwFlags);
    utils::endian::ByteSwap(header.dwHeight);
    utils::endian::ByteSwap(header.dwWidth);
    utils::endian::ByteSwap(header.dwPitchOrLinearSize);
    utils::endian::ByteSwap(header.dwDepth);
    utils::endian::ByteSwap(header.dwMipMapCount);

    for (int i = 0; i < 11; i++)
        utils::endian::ByteSwap(header.dwReserved1[i]);

    ByteSwapDDSPixelFormat(header.ddspf);
    utils::endian::ByteSwap(header.dwCaps);
    utils::endian::ByteSwap(header.dwCaps2);
    utils::endian::ByteSwap(header.dwCaps3);
    utils::endian::ByteSwap(header.dwCaps4);
    utils::endian::ByteSwap(header.dwReserved2);
}

bool PopulateDdsPixelFormat(DDS_PIXELFORMAT &pixelFormat, uint32_t gpuFormat)
{
    memset(&pixelFormat, 0, sizeof(pixelFormat));
    pixelFormat.dwSize = DDS_PIXEL_FORMAT_SIZE;

    switch (gpuFormat)
    {
    case GPUTEXTUREFORMAT_DXT1:
        pixelFormat.dwFlags = DDPF_FOURCC;
        pixelFormat.dwFourCC = DXT1_FOURCC;
        return true;
    case GPUTEXTUREFORMAT_DXT2_3:
        pixelFormat.dwFlags = DDPF_FOURCC;
        pixelFormat.dwFourCC = DXT3_FOURCC;
        return true;
    case GPUTEXTUREFORMAT_DXT4_5:
        pixelFormat.dwFlags = DDPF_FOURCC;
        pixelFormat.dwFourCC = DXT5_FOURCC;
        return true;
    case GPUTEXTUREFORMAT_DXN:
        pixelFormat.dwFlags = DDPF_FOURCC;
        pixelFormat.dwFourCC = DXN_FOURCC;
        return true;
    case GPUTEXTUREFORMAT_8:
        pixelFormat.dwFlags = DDPF_LUMINANCE;
        pixelFormat.dwRGBBitCount = 8;
        pixelFormat.dwRBitMask = 0x000000FF;
        return true;
    case GPUTEXTUREFORMAT_8_8:
        pixelFormat.dwFlags = DDPF_LUMINANCE | DDPF_ALPHAPIXELS;
        pixelFormat.dwRGBBitCount = 16;
        pixelFormat.dwRBitMask = 0x000000FF;
        pixelFormat.dwGBitMask = 0x0000FF00;
        return true;
    case GPUTEXTUREFORMAT_8_8_8_8:
        pixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
        pixelFormat.dwRGBBitCount = 32;
        pixelFormat.dwRBitMask = 0x00FF0000;
        pixelFormat.dwGBitMask = 0x0000FF00;
        pixelFormat.dwBBitMask = 0x000000FF;
        pixelFormat.dwABitMask = 0xFF000000;
        return true;
    default:
        return false;
    }
}
} // namespace

bool CreateDdsHeader(DDS_HEADER &header, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount,
                     uint32_t pitchOrLinearSize, uint32_t caps, uint32_t caps2, uint32_t gpuFormat)
{
    memset(&header, 0, sizeof(header));
    header.dwSize = DDS_HEADER_SIZE;
    header.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE;
    header.dwHeight = height;
    header.dwWidth = width;
    header.dwPitchOrLinearSize = pitchOrLinearSize;
    header.dwDepth = depth;
    header.dwMipMapCount = mipMapCount;
    header.dwCaps = caps;
    header.dwCaps2 = caps2;

    return PopulateDdsPixelFormat(header.ddspf, gpuFormat);
}

void WriteDdsHeader(std::ofstream &file, DDS_HEADER header)
{
    uint32_t magic = DDS_MAGIC;
    utils::endian::ByteSwap(magic);
    ByteSwapDDSHeader(header);

    file.write(reinterpret_cast<const char *>(&magic), sizeof(magic));
    file.write(reinterpret_cast<const char *>(&header), sizeof(header));
}
} // namespace image
