#pragma once

#include <cstddef>

namespace codxe
{

template <typename T, size_t MaxCount> struct FixedVector
{
    T items[MaxCount];
    size_t count;

    void Clear()
    {
        count = 0;
    }

    bool PushBack(const T &value)
    {
        if (count >= MaxCount)
            return false;

        items[count++] = value;
        return true;
    }

    void RemoveAt(size_t index)
    {
        if (index >= count)
            return;

        for (size_t i = index; i + 1 < count; ++i)
        {
            items[i] = items[i + 1];
        }

        --count;
    }

    T *Begin()
    {
        return items;
    }

    T *End()
    {
        return items + count;
    }

    const T *Begin() const
    {
        return items;
    }

    const T *End() const
    {
        return items + count;
    }
};

} // namespace codxe
