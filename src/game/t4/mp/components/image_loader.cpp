#include "pch.h"
#include "common/config.h"
#include "image_loader.h"
#include "image/dds_loader.h"
#include "image/xenos_texture.h"

namespace
{
namespace game = t4::mp;

const int CON_CHANNEL_ERROR = 1;
const int CON_CHANNEL_CONSOLEONLY = 7;

std::set<std::string> g_streamedImageReplacements;

typedef image::DdsImage DDSImage;

DDSImage ReadDDSFile(const std::string &filepath)
{
    return image::LoadDdsFromFile(filepath);
}

std::string GetReplacementPath(const char *imageName)
{
    return Config::GetModBasePath() + "\\images\\" + imageName + ".dds";
}

bool ValidateDDSHeader(const game::GfxImage *image, const DDSImage &ddsImage, const std::string &path,
                       GPUTEXTUREFORMAT *ddsFormat)
{
    if (ddsImage.data.empty())
    {
        game::Com_PrintError(CON_CHANNEL_ERROR, "Image_Replace: Failed to load DDS file for image '%s': %s\n",
                             image->name, path.c_str());
        return false;
    }

    if (ddsImage.header.dwSize != image::DDS_HEADER_SIZE ||
        ddsImage.header.ddspf.dwSize != image::DDS_PIXEL_FORMAT_SIZE)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' has an invalid DDS header: size=%u pixelFormatSize=%u\n",
                             image->name, ddsImage.header.dwSize, ddsImage.header.ddspf.dwSize);
        return false;
    }

    if (!ddsImage.GetGpuFormat(ddsFormat))
    {
        game::Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' has an unsupported DDS format: flags=0x%X fourCC=0x%X\n",
                             image->name, ddsImage.header.ddspf.dwFlags, ddsImage.header.ddspf.dwFourCC);
        return false;
    }

    if (static_cast<uint32_t>(image->texture.basemap->Format.DataFormat) != static_cast<uint32_t>(*ddsFormat))
    {
        game::Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' format does not match DDS file: Expected %u, Got %u\n",
                             image->name, static_cast<uint32_t>(image->texture.basemap->Format.DataFormat),
                             static_cast<uint32_t>(*ddsFormat));
        return false;
    }

    return true;
}

size_t CalculateRequiredLinearDataSize(uint32_t width, uint32_t height, GPUTEXTUREFORMAT format, uint32_t firstMipLevel,
                                       uint32_t levelCount, uint32_t faceCount)
{
    size_t requiredSize = 0;

    for (uint32_t localMipLevel = 0; localMipLevel < levelCount; ++localMipLevel)
    {
        const uint32_t mipLevel = firstMipLevel + localMipLevel;
        const uint32_t levelSize = image::xenos_texture::CalculateLinearLevelSize(width, height, mipLevel, format);
        if (levelSize == 0)
            return 0;

        requiredSize += static_cast<size_t>(levelSize) * faceCount;
    }

    return requiredSize;
}

uint32_t CalculateDDSMipOffset(uint32_t width, uint32_t height, GPUTEXTUREFORMAT format, uint32_t mipLevel)
{
    uint32_t offset = 0;

    for (uint32_t currentMip = 0; currentMip < mipLevel; ++currentMip)
    {
        const uint32_t levelSize = image::xenos_texture::CalculateLinearLevelSize(width, height, currentMip, format);
        if (levelSize == 0)
            return 0;

        offset += levelSize;
    }

    return offset;
}

size_t CalculateRequiredMipTextureBytes(uint32_t width, uint32_t height, GPUTEXTUREFORMAT format,
                                        uint32_t firstMipLevel, uint32_t levelCount, uint32_t faceCount)
{
    size_t requiredSize = 0;

    for (uint32_t mipLevel = firstMipLevel; mipLevel < levelCount; ++mipLevel)
    {
        const uint32_t levelSize = image::xenos_texture::CalculateTiledLevelSize(width, height, mipLevel, format, 0u);
        if (levelSize == 0)
            return 0;

        requiredSize += static_cast<size_t>(levelSize) * faceCount;
    }

    return requiredSize;
}

bool Validate2DReplacementData(const game::GfxImage *image, const DDSImage &ddsImage, GPUTEXTUREFORMAT format,
                               uint32_t ddsFirstMipLevel, uint32_t replacementLevelCount, size_t *requiredDDSSize,
                               size_t *requiredTextureBytes)
{
    const size_t ddsMipOffset =
        CalculateDDSMipOffset(ddsImage.header.dwWidth, ddsImage.header.dwHeight, format, ddsFirstMipLevel);
    const size_t requiredLinearSize = CalculateRequiredLinearDataSize(
        ddsImage.header.dwWidth, ddsImage.header.dwHeight, format, ddsFirstMipLevel, replacementLevelCount, 1u);
    *requiredDDSSize = ddsMipOffset + requiredLinearSize;
    if (requiredLinearSize == 0 || (ddsFirstMipLevel > 0 && ddsMipOffset == 0))
        return false;

    if (ddsImage.data.size() < *requiredDDSSize)
        return false;

    const uint32_t baseSize =
        image::xenos_texture::CalculateBaseSize(image->texture.basemap, image->width, image->height, 1u);
    const size_t mipBytes =
        CalculateRequiredMipTextureBytes(image->width, image->height, format, 1u, replacementLevelCount, 1u);
    *requiredTextureBytes = static_cast<size_t>(baseSize) + mipBytes;
    const int cardMemory = image->cardMemory.platform[0];
    if (cardMemory > 0 && *requiredTextureBytes > static_cast<size_t>(cardMemory))
        return false;

    return true;
}

bool ValidateCubeReplacementData(const game::GfxImage *image, const DDSImage &ddsImage, GPUTEXTUREFORMAT format,
                                 uint32_t faceSize, uint32_t tiledBaseSize, size_t *requiredDDSSize)
{
    *requiredDDSSize = static_cast<size_t>(faceSize) * 6u;
    if (faceSize == 0 || *requiredDDSSize == 0)
        return false;

    if (ddsImage.data.size() < *requiredDDSSize)
        return false;

    const int cardMemory = image->cardMemory.platform[0];
    if (cardMemory > 0 && static_cast<size_t>(tiledBaseSize) > static_cast<size_t>(cardMemory))
        return false;

    return true;
}

bool Image_Replace_2D(game::GfxImage *image, const DDSImage &ddsImage, uint32_t ddsFirstMipLevel)
{
    if (image->mapType != game::MAPTYPE_2D)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' is not a 2D map!\n", image->name);
        return false;
    }

    const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(image->texture.basemap->Format.DataFormat);
    const uint32_t levelCount = image::xenos_texture::GetTextureLevelCount(image->texture.basemap);
    const uint32_t mipTailBaseLevel = image->texture.basemap->Format.PackedMips != 0
                                          ? image::xenos_texture::GetMipTailBaseLevel(image->width, image->height)
                                          : levelCount;
    const uint32_t ddsMipCount = ddsImage.GetMipCount();
    if (ddsFirstMipLevel >= ddsMipCount)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' replacement DDS has no mip %u: mipCount=%u\n", image->name,
                             ddsFirstMipLevel, ddsMipCount);
        return false;
    }

    const uint32_t replaceLevelCount = min(levelCount, ddsMipCount - ddsFirstMipLevel);
    const uint32_t nonPackedLevelCount = max(1u, min(replaceLevelCount, mipTailBaseLevel));
    unsigned char *baseData = image::xenos_texture::GetTextureBase(image->texture.basemap, image->pixels);
    unsigned char *mipData = image::xenos_texture::GetTextureMipBase(image->texture.basemap, baseData, image->width,
                                                                     image->height, format, 1u);

    size_t requiredDDSSize = 0;
    size_t requiredTextureBytes = 0;
    if (!Validate2DReplacementData(image, ddsImage, format, ddsFirstMipLevel, nonPackedLevelCount, &requiredDDSSize,
                                   &requiredTextureBytes))
    {
        if (requiredDDSSize == 0)
        {
            game::Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' has unsupported replacement format %u!\n", image->name,
                                 static_cast<uint32_t>(format));
        }
        else if (ddsImage.data.size() < requiredDDSSize)
        {
            game::Com_PrintError(CON_CHANNEL_ERROR,
                                 "Image '%s' DDS data is too small: have=%u need=%u for %u mip levels\n", image->name,
                                 static_cast<unsigned int>(ddsImage.data.size()),
                                 static_cast<unsigned int>(requiredDDSSize), nonPackedLevelCount);
        }
        else
        {
            game::Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' replacement exceeds texture memory: have=%u need=%u\n",
                                 image->name, static_cast<unsigned int>(image->cardMemory.platform[0]),
                                 static_cast<unsigned int>(requiredTextureBytes));
        }

        return false;
    }

    if (baseData == NULL || mipData == NULL)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR, "Image_Replace_2D: Image '%s' has no valid texture memory!\n",
                             image->name);
        return false;
    }

    uint32_t ddsOffset =
        CalculateDDSMipOffset(ddsImage.header.dwWidth, ddsImage.header.dwHeight, format, ddsFirstMipLevel);

    for (uint32_t localMipLevel = 0; localMipLevel < nonPackedLevelCount; localMipLevel++)
    {
        const uint32_t ddsMipLevel = ddsFirstMipLevel + localMipLevel;
        const uint32_t rowPitch =
            image::xenos_texture::CalculateLinearRowPitch(ddsImage.header.dwWidth, ddsMipLevel, format);
        const uint32_t ddsMipLevelSize = image::xenos_texture::CalculateLinearLevelSize(
            ddsImage.header.dwWidth, ddsImage.header.dwHeight, ddsMipLevel, format);
        const uint32_t tiledMipLevelSize = image::xenos_texture::CalculateTiledLevelSize(
            image->width, image->height, localMipLevel, format, image->texture.basemap->Format.Pitch);

        if (ddsMipLevelSize == 0 || tiledMipLevelSize == 0 || rowPitch == 0)
        {
            game::Com_PrintError(CON_CHANNEL_ERROR,
                                 "Image_Replace_2D: Unsupported format %u for image '%s' mip level %u\n",
                                 image->texture.basemap->Format.DataFormat, image->name, localMipLevel);
            return false;
        }

        if (static_cast<size_t>(ddsOffset) + ddsMipLevelSize > ddsImage.data.size())
        {
            game::Com_PrintError(CON_CHANNEL_ERROR, "Image_Replace_2D: Image '%s' mip level %u exceeds DDS data size\n",
                                 image->name, ddsMipLevel);
            return false;
        }

        std::vector<uint8_t> levelData(ddsImage.data.begin() + ddsOffset,
                                       ddsImage.data.begin() + ddsOffset + ddsMipLevelSize);
        image::xenos_texture::ApplyGpuEndian(&levelData[0], levelData.size(),
                                             static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

        unsigned char *destination = baseData;
        if (localMipLevel > 0)
        {
            destination = mipData + image::xenos_texture::CalculateMipLevelOffset(image->width, image->height,
                                                                                  localMipLevel, format, 1u);
        }

        std::vector<uint8_t> tiledData(tiledMipLevelSize);
        if (!image::xenos_texture::TileTextureLevel(image->width, image->height, localMipLevel, format,
                                                    image->texture.basemap->Format.Pitch, &tiledData[0],
                                                    tiledData.size(), &levelData[0], levelData.size(), rowPitch))
        {
            game::Com_PrintError(CON_CHANNEL_ERROR, "Image_Replace_2D: Failed to tile mip level %u for image '%s'\n",
                                 localMipLevel, image->name);
            return false;
        }

        memcpy(destination, &tiledData[0], tiledMipLevelSize);
        ddsOffset += ddsMipLevelSize;
    }

    return true;
}

bool Image_Replace_Cube(game::GfxImage *image, const DDSImage &ddsImage)
{
    if (image->mapType != game::MAPTYPE_CUBE)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' is not a cube map!\n", image->name);
        return false;
    }

    const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(image->texture.basemap->Format.DataFormat);
    const uint32_t faceSize = image::xenos_texture::CalculateLinearLevelSize(image->width, image->height, 0u, format);
    const uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(image->width, 0u, format);
    const uint32_t tiledFaceSize = image::xenos_texture::CalculateTiledLevelSize(
        image->width, image->height, 0u, format, image->texture.basemap->Format.Pitch);
    const uint32_t tiledBaseSize =
        image::xenos_texture::CalculateBaseSize(image->texture.basemap, image->width, image->height, 6u);
    unsigned char *baseData = image::xenos_texture::GetTextureBase(image->texture.basemap, image->pixels);

    if (faceSize == 0 || rowPitch == 0 || tiledFaceSize == 0 || tiledBaseSize < tiledFaceSize * 6u)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' has unsupported format %u!\n", image->name,
                             static_cast<uint32_t>(format));
        return false;
    }

    if (baseData == NULL)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR, "Image_Replace_Cube: Image '%s' has no valid texture memory!\n",
                             image->name);
        return false;
    }

    size_t requiredDDSSize = 0;
    if (!ValidateCubeReplacementData(image, ddsImage, format, faceSize, tiledBaseSize, &requiredDDSSize))
    {
        if (requiredDDSSize == 0)
        {
            game::Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' has unsupported cube replacement format %u!\n",
                                 image->name, static_cast<uint32_t>(format));
        }
        else if (ddsImage.data.size() < requiredDDSSize)
        {
            game::Com_PrintError(CON_CHANNEL_ERROR,
                                 "Image_Replace_Cube: Image '%s' DDS is too small for 6 faces: have=%u need=%u\n",
                                 image->name, static_cast<unsigned int>(ddsImage.data.size()),
                                 static_cast<unsigned int>(requiredDDSSize));
        }
        else
        {
            game::Com_PrintError(CON_CHANNEL_ERROR,
                                 "Image '%s' cube replacement exceeds texture memory: have=%u "
                                 "need=%u\n",
                                 image->name, static_cast<unsigned int>(image->cardMemory.platform[0]), tiledBaseSize);
        }

        return false;
    }

    for (uint32_t faceIndex = 0; faceIndex < 6u; faceIndex++)
    {
        const unsigned char *facePixels = &ddsImage.data[faceIndex * faceSize];
        unsigned char *faceDestination = baseData + (faceIndex * tiledFaceSize);
        std::vector<uint8_t> tiledData(tiledFaceSize);

        if (!image::xenos_texture::TileTextureLevel(image->width, image->height, 0u, static_cast<uint32_t>(format),
                                                    image->texture.basemap->Format.Pitch, &tiledData[0],
                                                    tiledData.size(), facePixels, faceSize, rowPitch))
        {
            game::Com_PrintError(CON_CHANNEL_ERROR, "Image_Replace_Cube: Failed to tile image '%s' face %u\n",
                                 image->name, faceIndex);
            return false;
        }

        image::xenos_texture::ApplyGpuEndian(&tiledData[0], tiledData.size(),
                                             static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));
        memcpy(faceDestination, &tiledData[0], tiledFaceSize);
    }

    return true;
}

void Image_Replace(game::GfxImage *image)
{
    if (image == NULL || image->name == NULL || image->texture.basemap == NULL)
        return;

    const std::string replacementPath = GetReplacementPath(image->name);
    if (!filesystem::file_exists(replacementPath))
        return;

    DDSImage ddsImage = ReadDDSFile(replacementPath);
    GPUTEXTUREFORMAT ddsFormat;
    if (!ValidateDDSHeader(image, ddsImage, replacementPath, &ddsFormat))
        return;

    const bool ddsIsCubemap = ddsImage.IsCubemap();
    const bool ddsMatchesImageDimensions =
        image->width == ddsImage.header.dwWidth && image->height == ddsImage.header.dwHeight;
    const bool ddsMatchesStreamDimensions = image->streaming && image->mapType == game::MAPTYPE_2D && !ddsIsCubemap &&
                                            ddsImage.header.dwWidth == static_cast<uint32_t>(image->width) * 2u &&
                                            ddsImage.header.dwHeight == static_cast<uint32_t>(image->height) * 2u;
    uint32_t ddsFirstMipLevel = 0;

    if (image->mapType == game::MAPTYPE_2D && ddsIsCubemap)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' is 2D but replacement DDS is a cubemap!\n", image->name);
        return;
    }

    if (image->streaming && image->mapType == game::MAPTYPE_2D)
    {
        if (!ddsMatchesStreamDimensions)
        {
            game::Com_PrintError(CON_CHANNEL_ERROR,
                                 "Streamed image '%s' replacement must include the streamed mip: expected=%ux%u "
                                 "got=%ux%u %s\n",
                                 image->name, static_cast<uint32_t>(image->width) * 2u,
                                 static_cast<uint32_t>(image->height) * 2u, ddsImage.header.dwWidth,
                                 ddsImage.header.dwHeight, replacementPath.c_str());
            return;
        }

        const uint32_t ddsMipCount = ddsImage.GetMipCount();
        if (ddsMipCount < 2u)
        {
            game::Com_PrintError(CON_CHANNEL_ERROR,
                                 "Image '%s' replacement DDS starts at the streamed mip but has no resident mip: "
                                 "%ux%u mipCount=%u\n",
                                 image->name, ddsImage.header.dwWidth, ddsImage.header.dwHeight, ddsMipCount);
            return;
        }

        ddsFirstMipLevel = 1u;
    }
    else if (!ddsMatchesImageDimensions)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR,
                             "Image '%s' dimensions do not match DDS file: image=%ux%u dds=%ux%u "
                             "%s\n",
                             image->name, image->width, image->height, ddsImage.header.dwWidth,
                             ddsImage.header.dwHeight, replacementPath.c_str());
        return;
    }

    if (image->mapType == game::MAPTYPE_CUBE && !ddsIsCubemap)
    {
        const uint32_t faceSize =
            image::xenos_texture::CalculateLinearLevelSize(image->width, image->height, 0u, ddsFormat);
        if (faceSize == 0 || ddsImage.data.size() < static_cast<size_t>(faceSize) * 6u)
        {
            game::Com_PrintError(CON_CHANNEL_ERROR,
                                 "Image '%s' is a cubemap but replacement DDS is not a valid 6-face cubemap!\n",
                                 image->name);
            return;
        }

        game::Com_Printf(CON_CHANNEL_CONSOLEONLY,
                         "Image '%s' replacement DDS has no cubemap caps but contains enough data for 6 sequential "
                         "faces; accepting it.\n",
                         image->name);
    }

    bool replaced = false;
    if (image->mapType == game::MAPTYPE_2D)
        replaced = Image_Replace_2D(image, ddsImage, ddsFirstMipLevel);
    else if (image->mapType == game::MAPTYPE_CUBE)
        replaced = Image_Replace_Cube(image, ddsImage);
    else
        game::Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' is not a 2D or cube map!\n", image->name);

    if (replaced)
    {
        if (ddsFirstMipLevel > 0)
            g_streamedImageReplacements.insert(image->name);

        game::Com_Printf(CON_CHANNEL_CONSOLEONLY, "Replaced image '%s'\n", image->name);
    }
}

void Load_images()
{
    g_streamedImageReplacements.clear();

    const int MAX_IMAGES = 2048;
    game::XAssetHeader assets[MAX_IMAGES];
    const int count = game::DB_GetAllXAssetOfType_FastFile(game::ASSET_TYPE_IMAGE, assets, MAX_IMAGES);
    for (int i = 0; i < count; i++)
    {
        game::GfxImage *image = assets[i].image;
        Image_Replace(image);
    }
}

uint32_t GetStreamPriority(double imageDistSq)
{
    if (imageDistSq > 0.0)
    {
        if (imageDistSq > 10000.0)
            return imageDistSq > 90000.0 ? 1u : 2u;

        return 3u;
    }

    return 5u;
}

bool FinalizeStreamReplacement(game::StreamAllocBlockInfo *block, uint32_t streamSlot, game::GfxImage *image,
                               unsigned char *destination, uint32_t streamPriority)
{
    if (streamPriority == 5u)
    {
        while (!game::RB_StreamQueueCommandSetHighMip(image, destination))
        {
        }
    }

    game::R_StreamAlloc_SetImage(block, static_cast<__int16>(streamSlot), image);
    return true;
}

bool R_StreamLoadImageReplacement(game::GfxImage *image, double imageDistSq)
{
    if (image == NULL || image->name == NULL || image->texture.basemap == NULL || image->mapType != game::MAPTYPE_2D ||
        !image->streaming)
    {
        return false;
    }

    if (g_streamedImageReplacements.find(image->name) == g_streamedImageReplacements.end())
        return false;

    const std::string replacementPath = GetReplacementPath(image->name);
    if (!filesystem::file_exists(replacementPath))
    {
        game::Com_PrintError(CON_CHANNEL_ERROR,
                             "R_StreamLoadImageReplacement: Blocking stock stream for replaced "
                             "image '%s' because DDS is missing\n",
                             image->name);
        return true;
    }

    DDSImage ddsImage = ReadDDSFile(replacementPath);
    GPUTEXTUREFORMAT ddsFormat;
    if (!ValidateDDSHeader(image, ddsImage, replacementPath, &ddsFormat))
    {
        game::Com_PrintError(CON_CHANNEL_ERROR,
                             "R_StreamLoadImageReplacement: Blocking stock stream for replaced image '%s'\n",
                             image->name);
        return true;
    }

    const uint32_t streamWidth = static_cast<uint32_t>(image->width) * 2u;
    const uint32_t streamHeight = static_cast<uint32_t>(image->height) * 2u;
    if (ddsImage.IsCubemap() || ddsImage.header.dwWidth != streamWidth || ddsImage.header.dwHeight != streamHeight)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR,
                             "R_StreamLoadImageReplacement: Image '%s' dimensions do not match streamed mip: "
                             "expected=%ux%u got=%ux%u\n",
                             image->name, streamWidth, streamHeight, ddsImage.header.dwWidth, ddsImage.header.dwHeight);
        return true;
    }

    if (ddsImage.GetMipCount() < 2u)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR,
                             "R_StreamLoadImageReplacement: Image '%s' replacement DDS must include stream and "
                             "resident mips: mipCount=%u\n",
                             image->name, ddsImage.GetMipCount());
        return true;
    }

    if (image->baseSize == 0 || image->baseSize > 0x3FFFFFFFu)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR, "R_StreamLoadImageReplacement: Image '%s' has invalid stream size %u\n",
                             image->name, image->baseSize);
        return true;
    }

    const uint32_t streamSize = image->baseSize * 4u;
    const uint32_t sourceSize =
        image::xenos_texture::CalculateLinearLevelSize(streamWidth, streamHeight, 0u, ddsFormat);
    const uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(streamWidth, 0u, ddsFormat);
    const uint32_t tiledSize =
        image::xenos_texture::CalculateTiledLevelSize(streamWidth, streamHeight, 0u, ddsFormat, 0u);

    if (sourceSize == 0 || rowPitch == 0 || tiledSize == 0)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR,
                             "R_StreamLoadImageReplacement: Image '%s' has unsupported replacement format %u\n",
                             image->name, static_cast<uint32_t>(ddsFormat));
        return true;
    }

    if (ddsImage.data.size() < sourceSize)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR,
                             "R_StreamLoadImageReplacement: Image '%s' DDS data is too small: have=%u need=%u\n",
                             image->name, static_cast<unsigned int>(ddsImage.data.size()), sourceSize);
        return true;
    }

    if (tiledSize != streamSize)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR,
                             "R_StreamLoadImageReplacement: Image '%s' stream size mismatch: expected=%u got=%u\n",
                             image->name, streamSize, tiledSize);
        return true;
    }

    game::StreamAllocBlockInfo *streamBlock = NULL;
    uint32_t streamSlot = 0;
    const uint32_t streamPriority = GetStreamPriority(imageDistSq);
    if (!game::R_StreamAlloc_Alloc(streamSize, streamPriority, &streamBlock, &streamSlot))
    {
        game::Com_PrintError(CON_CHANNEL_ERROR,
                             "R_StreamLoadImageReplacement: Failed to allocate stream memory for "
                             "image '%s' size=%u\n",
                             image->name, streamSize);
        return true;
    }

    unsigned char *destination = reinterpret_cast<unsigned char *>(*game::r_streamBufferBase + (streamSlot << 17u));
    if (destination == NULL)
    {
        game::Com_PrintError(CON_CHANNEL_ERROR, "R_StreamLoadImageReplacement: Image '%s' stream destination is null\n",
                             image->name);
        return true;
    }

    std::vector<uint8_t> buffer(ddsImage.data.begin(), ddsImage.data.begin() + sourceSize);
    image::xenos_texture::ApplyGpuEndian(&buffer[0], buffer.size(),
                                         static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

    if (!image::xenos_texture::TileTextureLevel(streamWidth, streamHeight, 0u, ddsFormat, 0u, destination, streamSize,
                                                &buffer[0], buffer.size(), rowPitch))
    {
        memset(destination, 0, streamSize);
        game::Com_PrintError(CON_CHANNEL_ERROR, "R_StreamLoadImageReplacement: Failed to tile streamed image '%s'\n",
                             image->name);
        FinalizeStreamReplacement(streamBlock, streamSlot, image, destination, streamPriority);
        return true;
    }

    FinalizeStreamReplacement(streamBlock, streamSlot, image, destination, streamPriority);
    game::Com_Printf(CON_CHANNEL_CONSOLEONLY, "Replaced streamed image '%s'\n", image->name);
    return true;
}

Detour CG_RegisterGraphics_Detour;
Detour R_StreamLoadImage_Detour;

void CG_RegisterGraphics_Hook(int localClientNum, const char *mapname)
{
    CG_RegisterGraphics_Detour.GetOriginal<decltype(game::CG_RegisterGraphics)>()(localClientNum, mapname);
    Load_images();
}

void R_StreamLoadImage_Hook(game::GfxImage *image, double imageDistSq)
{
    if (R_StreamLoadImageReplacement(image, imageDistSq))
        return;

    R_StreamLoadImage_Detour.GetOriginal<decltype(game::R_StreamLoadImage)>()(image, imageDistSq);
}
} // namespace

namespace t4
{
namespace mp
{
image_loader::image_loader()
{
    CG_RegisterGraphics_Detour = Detour(CG_RegisterGraphics, CG_RegisterGraphics_Hook);
    CG_RegisterGraphics_Detour.Install();

    R_StreamLoadImage_Detour = Detour(R_StreamLoadImage, R_StreamLoadImage_Hook);
    R_StreamLoadImage_Detour.Install();
}

image_loader::~image_loader()
{
    CG_RegisterGraphics_Detour.Remove();
    R_StreamLoadImage_Detour.Remove();
}
} // namespace mp
} // namespace t4
