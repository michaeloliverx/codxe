#include "preview_dds.h"

namespace image
{
namespace preview
{
namespace
{
uint32_t ReadLE(const unsigned char *data)
{
    return uint32_t(data[0]) | (uint32_t(data[1]) << 8) | (uint32_t(data[2]) << 16) | (uint32_t(data[3]) << 24);
}

bool Fail(const char **error, const char *message)
{
    if (error)
        *error = message;
    return false;
}
} // namespace

bool ReadDDS(const void *data, size_t size, const unsigned char **pixels, const char **error)
{
    if (pixels)
        *pixels = NULL;
    if (error)
        *error = NULL;
    if (!data || size < HEADER_SIZE)
        return Fail(error, "truncated DDS header");
    const unsigned char *bytes = static_cast<const unsigned char *>(data);
    if (ReadLE(bytes) != 0x20534444 || ReadLE(bytes + 4) != 124 || ReadLE(bytes + 76) != 32)
        return Fail(error, "invalid DDS header");
    if ((ReadLE(bytes + 80) & 4) == 0 || ReadLE(bytes + 84) != 0x31545844)
        return Fail(error, "preview requires a legacy DXT1 header");
    if (ReadLE(bytes + 16) != WIDTH || ReadLE(bytes + 12) != HEIGHT)
        return Fail(error, "preview must be 512x512");
    if (ReadLE(bytes + 24) > 1 || ReadLE(bytes + 28) > 1 || ReadLE(bytes + 112) != 0 ||
        (ReadLE(bytes + 8) & 0x800000) != 0)
        return Fail(error, "preview must be a single 2D image without mipmaps");
    if (size != FILE_SIZE)
        return Fail(error, "incorrect DDS pixel data size");
    if (pixels)
        *pixels = bytes + HEADER_SIZE;
    return true;
}
} // namespace preview
} // namespace image
