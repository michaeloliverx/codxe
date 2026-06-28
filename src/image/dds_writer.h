#pragma once

#include "image/dds_types.h"

#include <iosfwd>

namespace image
{
bool CreateDdsHeader(DDS_HEADER &header, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipMapCount,
                     uint32_t pitchOrLinearSize, uint32_t caps, uint32_t caps2, uint32_t gpuFormat);
void WriteDdsHeader(std::ofstream &file, DDS_HEADER header);
} // namespace image
