#pragma once

#include <string>
#include <vector>

namespace usermaps
{
struct Entry
{
    std::string name;
    std::string displayName;
    std::string description;
    std::string previewPath;
};

const char *GetDirectory();
bool IsSafeName(const char *name);
bool IsMultiplayerName(const char *name);
std::string FastfilePath(const char *zoneName);
bool ReadMetadata(const std::string &directory, const std::string &map, std::string &name, std::string &description);
std::vector<Entry> ScanMultiplayer();
} // namespace usermaps
