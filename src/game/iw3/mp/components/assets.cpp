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

    const std::string filePath = map_ents::BuildPath(Config::GetModBasePath(), mapEnts->name);
    if (filePath.empty())
    {
        return;
    }

    HANDLE file = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        return;
    }

    const DWORD fileSize = GetFileSize(file, nullptr);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0 || fileSize > INT_MAX)
    {
        CloseHandle(file);
        return;
    }

    char *buffer = static_cast<char *>(Hunk_AllocAlignInternal(fileSize + 1, 1));
    if (!buffer)
    {
        CloseHandle(file);
        Com_PrintError(CON_CHANNEL_ERROR, "Failed to allocate MapEnts override for '%s'\n", mapEnts->name);
        return;
    }

    DWORD bytesRead = 0;
    if (!ReadFile(file, buffer, fileSize, &bytesRead, nullptr) || bytesRead != fileSize)
    {
        CloseHandle(file);
        Com_PrintError(CON_CHANNEL_ERROR, "Failed to read MapEnts override for '%s'\n", mapEnts->name);
        return;
    }

    CloseHandle(file);
    buffer[fileSize] = '\0';

    mapEnts->entityString = buffer;
    mapEnts->numEntityChars = static_cast<int>(fileSize);

    DbgPrint("[codxe][iw3] Loaded MapEnts override: %s (%u bytes at %p).\n", filePath.c_str(), fileSize, buffer);
}
} // namespace

void assets::OnAssetLink(XAsset *asset)
{
    switch (asset->type)
    {
    case ASSET_TYPE_MAP_ENTS:
        OverrideMapEnts(asset->header.mapEnts);
        break;
    }
}
} // namespace mp
} // namespace iw3
