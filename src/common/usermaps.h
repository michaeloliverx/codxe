#pragma once

#include <string>
#include <vector>
#include "arena.h"

namespace usermaps
{
struct Entry
{
    std::string name;
    std::string displayName;
    std::string description;
    std::string previewPath;
    arena::Fields fields;
};

const char *GetDirectory();
bool IsSafeName(const char *name);
bool IsMultiplayerName(const char *name);
std::string FastfilePath(const char *zoneName);
bool ReadMetadata(const std::string &directory, const std::string &map, std::string &name, std::string &description);
bool ReadMetadata(const std::string &directory, const std::string &map, arena::Fields &fields);
std::vector<Entry> ScanMultiplayer();
} // namespace usermaps
