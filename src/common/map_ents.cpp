#include "pch.h"

namespace map_ents
{
std::string GetFileNameForAssetName(const char *assetName)
{
    if (!assetName || assetName[0] == '\0')
        return std::string();

    std::string fileName = assetName;
    fileName += ".ents";
    return fileName;
}

std::string BuildPath(const char *directory, const char *assetName)
{
    if (!directory || directory[0] == '\0')
        return std::string();

    const std::string fileName = GetFileNameForAssetName(assetName);
    if (fileName.empty())
        return std::string();

    return filesystem::JoinPath(directory, fileName.c_str());
}
} // namespace map_ents
