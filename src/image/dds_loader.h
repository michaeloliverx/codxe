#pragma once

#include "image/dds_types.h"

#include <string>

namespace image
{
DdsImage LoadDdsFromFile(const std::string &path);
bool LoadDdsFromFile(const std::string &path, DdsImage *out);
uint32_t GetMipDimension(uint32_t dimension, uint32_t mipLevel);
} // namespace image
