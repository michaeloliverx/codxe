#include "pch.h"
#include "utils/endian.h"

#include <stdlib.h>

namespace utils
{
namespace endian
{
namespace
{
uint16_t ByteSwapped(uint16_t value)
{
    return static_cast<uint16_t>((value >> 8) | (value << 8));
}

int16_t ByteSwapped(int16_t value)
{
    return static_cast<int16_t>(ByteSwapped(static_cast<uint16_t>(value)));
}

uint32_t ByteSwapped(uint32_t value)
{
    return _byteswap_ulong(value);
}

int32_t ByteSwapped(int32_t value)
{
    return static_cast<int32_t>(ByteSwapped(static_cast<uint32_t>(value)));
}

uint64_t ByteSwapped(uint64_t value)
{
    return (static_cast<uint64_t>(ByteSwapped(static_cast<uint32_t>(value))) << 32) |
           ByteSwapped(static_cast<uint32_t>(value >> 32));
}

int64_t ByteSwapped(int64_t value)
{
    return static_cast<int64_t>(ByteSwapped(static_cast<uint64_t>(value)));
}
} // namespace

void ByteSwap(uint16_t &value)
{
    value = ByteSwapped(value);
}

void ByteSwap(int16_t &value)
{
    value = ByteSwapped(value);
}

void ByteSwap(uint32_t &value)
{
    value = ByteSwapped(value);
}

void ByteSwap(int32_t &value)
{
    value = ByteSwapped(value);
}

void ByteSwap(uint64_t &value)
{
    value = ByteSwapped(value);
}

void ByteSwap(int64_t &value)
{
    value = ByteSwapped(value);
}
} // namespace endian
} // namespace utils
