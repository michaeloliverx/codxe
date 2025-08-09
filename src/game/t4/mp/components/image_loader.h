#pragma once

#include "common.h"

// DDS Constants
const uint32_t DDS_MAGIC = MAKEFOURCC('D', 'D', 'S', ' ');
const uint32_t DDS_HEADER_SIZE = 124;
const uint32_t DDS_PIXEL_FORMAT_SIZE = 32;
const uint32_t DDSD_CAPS = 0x1;
const uint32_t DDSD_HEIGHT = 0x2;
const uint32_t DDSD_WIDTH = 0x4;
const uint32_t DDSD_PIXELFORMAT = 0x1000;
const uint32_t DDSD_LINEARSIZE = 0x80000;
const uint32_t DDPF_FOURCC = 0x4;
const uint32_t DDPF_RGB = 0x40;
const uint32_t DDPF_ALPHAPIXELS = 0x1;
const uint32_t DDSCAPS_TEXTURE = 0x1000;
const uint32_t DDSCAPS_MIPMAP = 0x400000;
const uint32_t DDPF_LUMINANCE = 0x20000;

// DDS Pixel Formats (FourCC Codes)
const uint32_t DXT1_FOURCC = MAKEFOURCC('D', 'X', 'T', '1');
const uint32_t DXT3_FOURCC = MAKEFOURCC('D', 'X', 'T', '3');
const uint32_t DXT5_FOURCC = MAKEFOURCC('D', 'X', 'T', '5');
const uint32_t DXN_FOURCC = MAKEFOURCC('A', 'T', 'I', '2'); // (DXN / BC5)

// Additional DDS Cubemap Flags
const uint32_t DDSCAPS2_CUBEMAP = 0x200;
const uint32_t DDSCAPS2_CUBEMAP_POSITIVEX = 0x400;
const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800;
const uint32_t DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000;
const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000;
const uint32_t DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000;
const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000;

struct DDSHeader
{
    uint32_t magic;
    uint32_t size;
    uint32_t flags;
    uint32_t height;
    uint32_t width;
    uint32_t pitchOrLinearSize;
    uint32_t depth;
    uint32_t mipMapCount;
    uint32_t reserved1[11];
    struct
    {
        uint32_t size;
        uint32_t flags;
        uint32_t fourCC;
        uint32_t rgbBitCount;
        uint32_t rBitMask;
        uint32_t gBitMask;
        uint32_t bBitMask;
        uint32_t aBitMask;
    } pixelFormat;
    uint32_t caps;
    uint32_t caps2;
    uint32_t caps3;
    uint32_t caps4;
    uint32_t reserved2;
};
static_assert(sizeof(DDSHeader) == 128, "");

struct DDSImage
{
    DDSHeader header;
    std::vector<uint8_t> data;
};

namespace t4
{
namespace mp
{

class ImageLoader : public Module
{

  public:
    ImageLoader();
    ~ImageLoader();
    const char *get_name() override
    {
        return "ImageLoader";
    };

  private:
    void DumpGfxImage(const GfxImage *image);
    void ImageLoader::ReplaceGfxImage(GfxImage *image);
    void ImageLoader::ReplaceGfxImage2d(GfxImage *image, const DDSImage &ddsImage);
    unsigned int ImageLoader::CalculateMipLevelSize(unsigned int width, unsigned int height, unsigned int mipLevel,
                                                    GPUTEXTUREFORMAT format);
};
} // namespace mp
} // namespace t4
