#include "pch.h"

namespace map_ents
{
std::string BuildPath(const char *directory, const char *assetName)
{
    if (!directory || directory[0] == '\0' || !assetName || assetName[0] == '\0')
    {
        return std::string();
    }

    std::string path = directory;
    path += '\\';
    path += assetName;
    path += ".ents";

    filesystem::NormalizePathSeparators(&path[0]);
    return path;
}
} // namespace map_ents
