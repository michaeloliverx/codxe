#pragma once

#include <cstddef>
#include <cstring>

namespace gsc
{
template <typename Handler> struct Entry
{
    const char *actionString;
    Handler actionFunc;
    __int32 type;
};

template <typename EntryType, std::size_t EntryCount> std::size_t Size(const EntryType (&)[EntryCount])
{
    return EntryCount;
}

template <typename EntryType, std::size_t EntryCount>
const EntryType *Find(const char *name, const EntryType (&entries)[EntryCount])
{
    if (!name)
        return nullptr;

    for (std::size_t i = 0; i < Size(entries); ++i)
    {
        if (_stricmp(name, entries[i].actionString) == 0)
            return &entries[i];
    }

    return nullptr;
}
} // namespace gsc
