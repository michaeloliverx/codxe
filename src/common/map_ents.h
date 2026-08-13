#pragma once

#include "pch.h"

namespace map_ents
{
std::string GetFileNameForAssetName(const char *assetName);
std::string BuildPath(const char *directory, const char *assetName);
} // namespace map_ents
