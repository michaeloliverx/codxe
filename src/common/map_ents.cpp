#include "pch.h"

namespace map_ents
{
std::string BuildPath(const char *directory, const char *assetName)
{
    if (!directory || directory[0] == '\0' || !assetName || assetName[0] == '\0')
    {
        return std::string();
    }

    std::string fileName = assetName;
    fileName += ".ents";

    return filesystem::JoinPath(directory, fileName.c_str());
}
} // namespace map_ents
