#include "pch.h"
#include "assets.h"

namespace iw3
{
namespace mp
{
namespace
{
void OverrideMapEnts(MapEnts *mapEnts)
{
    if (!mapEnts || !mapEnts->name || mapEnts->name[0] == ',')
    {
        return;
    }

    std::string filePath = Config::GetModBasePath();
    filePath += std::string("\\") + mapEnts->name + ".ents";
    std::replace(filePath.begin(), filePath.end(), '/', '\\');

    if (!filesystem::file_exists(filePath))
    {
        return;
    }

    const std::string entityString = filesystem::read_file_to_string(filePath);
    if (entityString.empty())
    {
        return;
    }

    const size_t bufferSize = entityString.size() + 1;
    char *buffer = static_cast<char *>(malloc(bufferSize));
    if (!buffer)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Failed to allocate MapEnts override for '%s'\n", mapEnts->name);
        return;
    }

    memcpy(buffer, entityString.c_str(), bufferSize);
    mapEnts->entityString = buffer;
}
} // namespace

void assets::OnAssetLink(XAsset *asset)
{
    if (asset->type != ASSET_TYPE_MAP_ENTS)
    {
        return;
    }

    OverrideMapEnts(asset->header.mapEnts);
}
} // namespace mp
} // namespace iw3
