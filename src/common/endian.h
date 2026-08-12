#pragma once

#include <stdint.h>

namespace endian
{
uint16_t ByteSwap(uint16_t value);
int16_t ByteSwap(int16_t value);
uint32_t ByteSwap(uint32_t value);
int32_t ByteSwap(int32_t value);
uint64_t ByteSwap(uint64_t value);
int64_t ByteSwap(int64_t value);
} // namespace endian
