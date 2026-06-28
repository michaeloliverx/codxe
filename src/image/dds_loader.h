#pragma once

#include "image/dds_types.h"

#include <string>

namespace image
{
DdsImage LoadDdsFromFile(const std::string &path);
bool LoadDdsFromFile(const std::string &path, DdsImage *out);
bool LoadDdsHeaderFromFile(const std::string &path, DDS_HEADER *outHeader, uint32_t *outDataSize);
bool LoadDdsDataRangeFromFile(const std::string &path, uint32_t dataOffset, void *buffer, uint32_t size);
uint32_t GetMipDimension(uint32_t dimension, uint32_t mipLevel);
} // namespace image
