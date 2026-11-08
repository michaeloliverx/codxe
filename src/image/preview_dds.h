#pragma once

#include <stddef.h>
#include <stdint.h>

namespace image
{
namespace preview
{
// Shared usermap preview contract: legacy DXT1 DDS, 512x512, one level.
const unsigned int WIDTH = 512;
const unsigned int HEIGHT = 512;
const size_t HEADER_SIZE = 128;
const size_t PIXEL_SIZE = WIDTH * HEIGHT / 2;
const size_t FILE_SIZE = HEADER_SIZE + PIXEL_SIZE;

// Returns a view into data. Reads little-endian fields explicitly on both hosts.
bool ReadDDS(const void *data, size_t size, const unsigned char **pixels, const char **error);
} // namespace preview
} // namespace image
