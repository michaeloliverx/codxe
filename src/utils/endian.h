#pragma once

#include <cstdint>

namespace utils
{
namespace endian
{
void ByteSwap(uint16_t &value);
void ByteSwap(int16_t &value);
void ByteSwap(uint32_t &value);
void ByteSwap(int32_t &value);
void ByteSwap(uint64_t &value);
void ByteSwap(int64_t &value);
} // namespace endian
} // namespace utils
