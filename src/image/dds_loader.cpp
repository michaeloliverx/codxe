#include "pch.h"
#include "image/dds_loader.h"
#include "utils/endian.h"

namespace image
{
namespace
{
const size_t DDS_FILE_HEADER_SIZE = sizeof(uint32_t) + sizeof(DDS_HEADER);

void SwapDDSHeaderEndian(DDS_HEADER &header)
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

    utils::endian::ByteSwap(header.ddspf.dwSize);
    utils::endian::ByteSwap(header.ddspf.dwFlags);
    utils::endian::ByteSwap(header.ddspf.dwFourCC);
    utils::endian::ByteSwap(header.ddspf.dwRGBBitCount);
    utils::endian::ByteSwap(header.ddspf.dwRBitMask);
    utils::endian::ByteSwap(header.ddspf.dwGBitMask);
    utils::endian::ByteSwap(header.ddspf.dwBBitMask);
    utils::endian::ByteSwap(header.ddspf.dwABitMask);

    utils::endian::ByteSwap(header.dwCaps);
    utils::endian::ByteSwap(header.dwCaps2);
    utils::endian::ByteSwap(header.dwCaps3);
    utils::endian::ByteSwap(header.dwCaps4);
    utils::endian::ByteSwap(header.dwReserved2);
}

bool IsValidDdsImage(const DdsImage &image)
{
    return image.header.dwSize == DDS_HEADER_SIZE && image.header.ddspf.dwSize == DDS_PIXEL_FORMAT_SIZE &&
           !image.data.empty();
}
} // namespace

bool DdsImage::IsCubemap() const
{
    const uint32_t cubemapFaces = DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_NEGATIVEX | DDSCAPS2_CUBEMAP_POSITIVEY |
                                  DDSCAPS2_CUBEMAP_NEGATIVEY | DDSCAPS2_CUBEMAP_POSITIVEZ | DDSCAPS2_CUBEMAP_NEGATIVEZ;

    return (header.dwCaps2 & DDSCAPS2_CUBEMAP) != 0 || (header.dwCaps2 & cubemapFaces) == cubemapFaces;
}

uint32_t DdsImage::GetMipCount() const
{
    return max(1u, static_cast<uint32_t>(header.dwMipMapCount));
}

bool DdsImage::GetGpuFormat(GPUTEXTUREFORMAT *format) const
{
    if (format == NULL)
        return false;

    if ((header.ddspf.dwFlags & DDPF_FOURCC) != 0)
    {
        switch (header.ddspf.dwFourCC)
        {
        case DXT1_FOURCC:
            *format = GPUTEXTUREFORMAT_DXT1;
            return true;
        case DXT3_FOURCC:
            *format = GPUTEXTUREFORMAT_DXT2_3;
            return true;
        case DXT5_FOURCC:
            *format = GPUTEXTUREFORMAT_DXT4_5;
            return true;
        case DXN_FOURCC:
            *format = GPUTEXTUREFORMAT_DXN;
            return true;
        default:
            return false;
        }
    }

    if ((header.ddspf.dwFlags & DDPF_LUMINANCE) != 0)
    {
        if (header.ddspf.dwRGBBitCount == 8 && header.ddspf.dwRBitMask == 0x000000FF)
        {
            *format = GPUTEXTUREFORMAT_8;
            return true;
        }

        if ((header.ddspf.dwFlags & DDPF_ALPHAPIXELS) != 0 && header.ddspf.dwRGBBitCount == 16 &&
            header.ddspf.dwRBitMask == 0x000000FF && header.ddspf.dwABitMask == 0x0000FF00)
        {
            *format = GPUTEXTUREFORMAT_8_8;
            return true;
        }
    }

    if ((header.ddspf.dwFlags & DDPF_RGB) != 0 && (header.ddspf.dwFlags & DDPF_ALPHAPIXELS) != 0 &&
        header.ddspf.dwRGBBitCount == 32 && header.ddspf.dwRBitMask == 0x00FF0000 &&
        header.ddspf.dwGBitMask == 0x0000FF00 && header.ddspf.dwBBitMask == 0x000000FF &&
        header.ddspf.dwABitMask == 0xFF000000)
    {
        *format = GPUTEXTUREFORMAT_8_8_8_8;
        return true;
    }

    return false;
}

DdsImage LoadDdsFromFile(const std::string &path)
{
    DdsImage image = {};
    std::ifstream file(path.c_str(), std::ios::binary);

    if (!file.is_open())
        return image;

    uint32_t magic = 0;
    file.read(reinterpret_cast<char *>(&magic), sizeof(magic));
    if (!file || file.gcount() != sizeof(magic))
        return DdsImage();

    utils::endian::ByteSwap(magic);
    if (magic != DDS_MAGIC)
        return DdsImage();

    file.read(reinterpret_cast<char *>(&image.header), sizeof(DDS_HEADER));
    if (!file || file.gcount() != sizeof(DDS_HEADER))
        return DdsImage();

    SwapDDSHeaderEndian(image.header);

    file.seekg(0, std::ios::end);
    const std::streampos fileSize = file.tellg();
    if (fileSize == std::streampos(-1) || fileSize < static_cast<std::streampos>(DDS_FILE_HEADER_SIZE))
        return DdsImage();

    file.seekg(DDS_FILE_HEADER_SIZE, std::ios::beg);
    const size_t dataSize = static_cast<size_t>(fileSize) - DDS_FILE_HEADER_SIZE;
    image.data.resize(dataSize);

    if (dataSize > 0)
    {
        file.read(reinterpret_cast<char *>(&image.data[0]), dataSize);
        if (!file || static_cast<size_t>(file.gcount()) != dataSize)
            return DdsImage();
    }

    return image;
}

bool LoadDdsFromFile(const std::string &path, DdsImage *out)
{
    if (out == NULL)
        return false;

    *out = LoadDdsFromFile(path);
    return IsValidDdsImage(*out);
}

uint32_t GetMipDimension(uint32_t dimension, uint32_t mipLevel)
{
    return max(1u, dimension >> mipLevel);
}
} // namespace image
