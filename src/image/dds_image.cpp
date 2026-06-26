#include "pch.h"
#include "image/dds_image.h"

namespace image
{
namespace
{
const uint32_t DDS_MAGIC = MAKEFOURCC('D', 'D', 'S', ' ');
const uint32_t DDPF_ALPHAPIXELS = 0x1;
const uint32_t DDPF_FOURCC = 0x4;
const uint32_t DDPF_RGB = 0x40;
const uint32_t DDPF_LUMINANCE = 0x20000;

const uint32_t DXT1_FOURCC = MAKEFOURCC('D', 'X', 'T', '1');
const uint32_t DXT3_FOURCC = MAKEFOURCC('D', 'X', 'T', '3');
const uint32_t DXT5_FOURCC = MAKEFOURCC('D', 'X', 'T', '5');
const uint32_t DXN_FOURCC = MAKEFOURCC('A', 'T', 'I', '2');

const uint32_t DDSCAPS2_CUBEMAP = 0x200;
const uint32_t DDSCAPS2_CUBEMAP_POSITIVEX = 0x400;
const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800;
const uint32_t DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000;
const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000;
const uint32_t DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000;
const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000;
const uint32_t DDSCAPS2_CUBEMAP_ALL_FACES = DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_NEGATIVEX |
                                            DDSCAPS2_CUBEMAP_POSITIVEY | DDSCAPS2_CUBEMAP_NEGATIVEY |
                                            DDSCAPS2_CUBEMAP_POSITIVEZ | DDSCAPS2_CUBEMAP_NEGATIVEZ;

void SwapDdsHeaderEndian(DdsHeader &header)
{
    header.magic = _byteswap_ulong(header.magic);
    header.size = _byteswap_ulong(header.size);
    header.flags = _byteswap_ulong(header.flags);
    header.height = _byteswap_ulong(header.height);
    header.width = _byteswap_ulong(header.width);
    header.pitchOrLinearSize = _byteswap_ulong(header.pitchOrLinearSize);
    header.depth = _byteswap_ulong(header.depth);
    header.mipMapCount = _byteswap_ulong(header.mipMapCount);

    for (int i = 0; i < 11; i++)
        header.reserved1[i] = _byteswap_ulong(header.reserved1[i]);

    header.pixelFormat.size = _byteswap_ulong(header.pixelFormat.size);
    header.pixelFormat.flags = _byteswap_ulong(header.pixelFormat.flags);
    header.pixelFormat.fourCC = _byteswap_ulong(header.pixelFormat.fourCC);
    header.pixelFormat.rgbBitCount = _byteswap_ulong(header.pixelFormat.rgbBitCount);
    header.pixelFormat.rBitMask = _byteswap_ulong(header.pixelFormat.rBitMask);
    header.pixelFormat.gBitMask = _byteswap_ulong(header.pixelFormat.gBitMask);
    header.pixelFormat.bBitMask = _byteswap_ulong(header.pixelFormat.bBitMask);
    header.pixelFormat.aBitMask = _byteswap_ulong(header.pixelFormat.aBitMask);

    header.caps = _byteswap_ulong(header.caps);
    header.caps2 = _byteswap_ulong(header.caps2);
    header.caps3 = _byteswap_ulong(header.caps3);
    header.caps4 = _byteswap_ulong(header.caps4);
    header.reserved2 = _byteswap_ulong(header.reserved2);
}
} // namespace

bool DdsImage::IsValid() const
{
    return header.magic == DDS_MAGIC && header.size == DDS_HEADER_SIZE &&
           header.pixelFormat.size == DDS_PIXEL_FORMAT_SIZE && !data.empty();
}

bool DdsImage::IsCubemap() const
{
    return (header.caps2 & DDSCAPS2_CUBEMAP) != 0 ||
           (header.caps2 & DDSCAPS2_CUBEMAP_ALL_FACES) == DDSCAPS2_CUBEMAP_ALL_FACES;
}

uint32_t DdsImage::GetMipCount() const
{
    return max(1u, static_cast<uint32_t>(header.mipMapCount));
}

bool DdsImage::GetGpuFormat(GPUTEXTUREFORMAT *format) const
{
    if (format == NULL)
        return false;

    if ((header.pixelFormat.flags & DDPF_FOURCC) != 0)
    {
        switch (header.pixelFormat.fourCC)
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

    if ((header.pixelFormat.flags & DDPF_LUMINANCE) != 0)
    {
        if (header.pixelFormat.rgbBitCount == 8 && header.pixelFormat.rBitMask == 0x000000FF)
        {
            *format = GPUTEXTUREFORMAT_8;
            return true;
        }

        if ((header.pixelFormat.flags & DDPF_ALPHAPIXELS) != 0 && header.pixelFormat.rgbBitCount == 16 &&
            header.pixelFormat.rBitMask == 0x000000FF && header.pixelFormat.gBitMask == 0x0000FF00)
        {
            *format = GPUTEXTUREFORMAT_8_8;
            return true;
        }
    }

    if ((header.pixelFormat.flags & DDPF_RGB) != 0 && (header.pixelFormat.flags & DDPF_ALPHAPIXELS) != 0 &&
        header.pixelFormat.rgbBitCount == 32 && header.pixelFormat.rBitMask == 0x00FF0000 &&
        header.pixelFormat.gBitMask == 0x0000FF00 && header.pixelFormat.bBitMask == 0x000000FF &&
        header.pixelFormat.aBitMask == 0xFF000000)
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

    file.read(reinterpret_cast<char *>(&image.header), sizeof(DdsHeader));
    if (!file || file.gcount() != sizeof(DdsHeader))
        return DdsImage();

    if (_byteswap_ulong(image.header.magic) != DDS_MAGIC)
        return DdsImage();

    SwapDdsHeaderEndian(image.header);

    file.seekg(0, std::ios::end);
    const std::streampos fileSize = file.tellg();
    if (fileSize == std::streampos(-1) || fileSize < static_cast<std::streampos>(sizeof(DdsHeader)))
        return DdsImage();

    file.seekg(sizeof(DdsHeader), std::ios::beg);
    const size_t dataSize = static_cast<size_t>(fileSize) - sizeof(DdsHeader);
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
    return out->IsValid();
}

uint32_t GetMipDimension(uint32_t dimension, uint32_t mipLevel)
{
    return max(1u, dimension >> mipLevel);
}
} // namespace image
