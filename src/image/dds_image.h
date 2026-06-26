#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace image
{
const uint32_t DDS_HEADER_SIZE = 124u;
const uint32_t DDS_PIXEL_FORMAT_SIZE = 32u;

struct DdsHeader
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

static_assert(sizeof(DdsHeader) == 128, "");

struct DdsImage
{
    DdsHeader header;
    std::vector<uint8_t> data;

    bool IsValid() const;
    bool IsCubemap() const;
    uint32_t GetMipCount() const;
    bool GetGpuFormat(GPUTEXTUREFORMAT *format) const;
};

DdsImage LoadDdsFromFile(const std::string &path);
bool LoadDdsFromFile(const std::string &path, DdsImage *out);
uint32_t GetMipDimension(uint32_t dimension, uint32_t mipLevel);
} // namespace image
