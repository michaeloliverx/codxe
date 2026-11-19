#pragma once

#include "usermaps.h"

namespace mod_metadata
{
struct Entry
{
    std::string name;
    std::string displayName;
    std::string description;
    std::string previewPath;
    usermaps::Credits credits;
};

Entry Read(const char *directory, const char *name);
} // namespace mod_metadata
