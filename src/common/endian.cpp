#include "pch.h"
#include "endian.h"

namespace endian
{
uint16_t ByteSwap(uint16_t value)
{
    return static_cast<uint16_t>((value >> 8) | (value << 8));
}

int16_t ByteSwap(int16_t value)
{
    return static_cast<int16_t>(ByteSwap(static_cast<uint16_t>(value)));
}

uint32_t ByteSwap(uint32_t value)
{
    return ((value & 0x000000FFu) << 24) | ((value & 0x0000FF00u) << 8) | ((value & 0x00FF0000u) >> 8) |
           ((value & 0xFF000000u) >> 24);
}

int32_t ByteSwap(int32_t value)
{
    return static_cast<int32_t>(ByteSwap(static_cast<uint32_t>(value)));
}

uint64_t ByteSwap(uint64_t value)
{
    return (static_cast<uint64_t>(ByteSwap(static_cast<uint32_t>(value))) << 32) |
           ByteSwap(static_cast<uint32_t>(value >> 32));
}

int64_t ByteSwap(int64_t value)
{
    return static_cast<int64_t>(ByteSwap(static_cast<uint64_t>(value)));
}
} // namespace endian
