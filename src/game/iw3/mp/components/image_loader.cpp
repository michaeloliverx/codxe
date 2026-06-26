#include "pch.h"
#include "common/config.h"
#include "command.h"
#include "image_loader.h"
#include "image/dds_loader.h"
#include "image/dds_writer.h"
#include "image/texture_layout.h"
#include "image/xenos_texture.h"

namespace
{
std::set<std::string> g_streamedImageReplacements;

std::string extract_filename(const char *filename)
{
    std::string path(filename);

    // Find last backslash '\' or forward slash '/'
    size_t lastSlash = path.find_last_of("\\/");
    size_t start = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;

    // Find last dot '.' (extension separator)
    size_t lastDot = path.find_last_of('.');
    size_t end = (lastDot == std::string::npos || lastDot < start) ? path.length() : lastDot;

    return path.substr(start, end - start);
}

bool Validate2DReplacementData(const iw3::mp::GfxImage *image, const image::DdsImage &ddsImage, GPUTEXTUREFORMAT format,
                               uint32_t ddsFirstMipLevel, uint32_t replacementLevelCount, size_t *requiredDDSSize,
                               size_t *requiredTextureBytes)
{
    const size_t ddsMipOffset =
        image::CalculateDdsMipOffset(ddsImage.header.dwWidth, ddsImage.header.dwHeight, format, ddsFirstMipLevel);
    const size_t requiredLinearSize = image::CalculateRequiredLinearDataSize(
        ddsImage.header.dwWidth, ddsImage.header.dwHeight, format, ddsFirstMipLevel, replacementLevelCount, 1u);
    *requiredDDSSize = ddsMipOffset + requiredLinearSize;
    if (requiredLinearSize == 0 || (ddsFirstMipLevel > 0 && ddsMipOffset == 0))
        return false;

    if (ddsImage.data.size() < *requiredDDSSize)
        return false;

    const uint32_t baseSize =
        image::xenos_texture::CalculateBaseSize(image->texture.basemap, image->width, image->height, 1u);
    const size_t mipBytes =
        image::CalculateRequiredMipTextureBytes(image->width, image->height, format, 1u, replacementLevelCount, 1u);
    *requiredTextureBytes = static_cast<size_t>(baseSize) + mipBytes;
    const int cardMemory = image->cardMemory.platform[0];
    if (cardMemory > 0 && *requiredTextureBytes > static_cast<size_t>(cardMemory))
        return false;

    return true;
}

bool ValidateCubeReplacementData(const iw3::mp::GfxImage *image, const image::DdsImage &ddsImage,
                                 GPUTEXTUREFORMAT format, uint32_t faceSize, uint32_t tiledBaseSize,
                                 size_t *requiredDDSSize)
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

} // namespace

namespace iw3
{
namespace mp
{
namespace
{
const char *HIGHMIP_DIR = "D:\\highmip";

std::string GetSanitizedImageName(const char *imageName)
{
    if (imageName == NULL)
        return std::string();

    std::string sanitizedName = imageName;
    sanitizedName.erase(
        std::remove_if(sanitizedName.begin(), sanitizedName.end(), [](char c) { return c == '*'; }),
        sanitizedName.end());
    return sanitizedName;
}

std::string GetImageDumpPath(const char *imageName)
{
    return std::string(DUMP_DIR) + "\\images\\" + GetSanitizedImageName(imageName) + ".dds";
}

bool ReadBinaryFile(const std::string &path, std::vector<uint8_t> *buffer)
{
    std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
    if (!file)
        return false;

    const std::streamsize size = file.tellg();
    if (size < 0)
        return false;

    file.seekg(0, std::ios::beg);
    buffer->resize(static_cast<size_t>(size));
    if (size == 0)
        return true;

    return file.read(reinterpret_cast<char *>(buffer->data()), size) != 0;
}

std::map<std::string, std::string> CollectHighMipFiles()
{
    std::map<std::string, std::string> highMipFiles;
    const std::vector<std::string> files = filesystem::list_files_in_directory(HIGHMIP_DIR);
    for (size_t i = 0; i < files.size(); ++i)
    {
        const std::string assetName = extract_filename(files[i].c_str());
        highMipFiles[assetName] = std::string(HIGHMIP_DIR) + "\\" + files[i];
    }

    return highMipFiles;
}

std::string FindHighMipPathForImage(const std::map<std::string, std::string> &highMipFiles, const char *imageName)
{
    if (imageName == NULL)
        return std::string();

    std::map<std::string, std::string>::const_iterator highMip = highMipFiles.find(imageName);
    if (highMip != highMipFiles.end())
        return highMip->second;

    const std::string sanitizedName = GetSanitizedImageName(imageName);
    highMip = highMipFiles.find(sanitizedName);
    if (highMip != highMipFiles.end())
        return highMip->second;

    return std::string();
}

bool Image_DumpHighMip(const GfxImage *image, const std::string &highMipPath)
{
    if (image->mapType != MAPTYPE_2D)
        return false;

    std::vector<uint8_t> highMipData;
    if (!ReadBinaryFile(highMipPath, &highMipData))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Could not read highmip for image '%s': %s\n", image->name,
                       highMipPath.c_str());
        return false;
    }

    const uint32_t width = static_cast<uint32_t>(image->width) * 2u;
    const uint32_t height = static_cast<uint32_t>(image->height) * 2u;
    const uint32_t format = image->texture.basemap->Format.DataFormat;
    const uint32_t linearLevelSize = image::xenos_texture::CalculateLinearLevelSize(width, height, 0u, format);
    const uint32_t tiledLevelSize = image::xenos_texture::CalculateTiledLevelSize(width, height, 0u, format, 0u);
    const uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(width, 0u, format);
    if (linearLevelSize == 0 || tiledLevelSize == 0 || rowPitch == 0)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Skipping image '%s': unsupported highmip format %u\n", image->name, format);
        return false;
    }

    if (highMipData.size() < tiledLevelSize)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Skipping image '%s': highmip data is too small (have %u, need %u)\n",
                       image->name, static_cast<unsigned int>(highMipData.size()), tiledLevelSize);
        return false;
    }

    image::DDS_HEADER header;
    if (!image::CreateDdsHeader(header, width, height, image->depth, 1u, linearLevelSize, image::DDSCAPS_TEXTURE, 0u,
                                format))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Skipping image '%s': unsupported highmip format %u\n", image->name, format);
        return false;
    }

    const std::string outputPath = GetImageDumpPath(image->name);
    std::ofstream outputFile(outputPath.c_str(), std::ios::binary);
    if (!outputFile)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Could not create DDS for image '%s': %s\n", image->name, outputPath.c_str());
        return false;
    }

    image::WriteDdsHeader(outputFile, header);

    std::vector<uint8_t> tiledData(highMipData.begin(), highMipData.begin() + tiledLevelSize);
    image::xenos_texture::ApplyGpuEndian(tiledData.data(), tiledData.size(),
                                         static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

    std::vector<uint8_t> linearData(linearLevelSize);
    if (!image::xenos_texture::UntileTextureLevel(width, height, 0u, format, 0u, linearData.data(), linearData.size(),
                                                  rowPitch, tiledData.data(), tiledData.size()))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Could not decode highmip for image '%s'\n", image->name);
        return false;
    }

    outputFile.write(reinterpret_cast<const char *>(linearData.data()), linearData.size());
    Com_Printf(CON_CHANNEL_CONSOLEONLY, "Dumped image '%s' (highmip)\n", image->name);
    return true;
}
} // namespace

void Image_Dump(const GfxImage *image, const std::string &highMipPath)
{
    // TODO: cleanup empty files if failed

    if (!image)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Skipping image: null asset\n");
        return;
    }

    if (image->name == NULL)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Skipping image: missing name\n");
        return;
    }

    if (!image->pixels || image->baseSize == 0)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Skipping image '%s': no pixel data\n", image->name);
        return;
    }

    if (image->mapType != MAPTYPE_2D && image->mapType != MAPTYPE_CUBE)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Skipping image '%s': unsupported map type %d\n", image->name,
                       image->mapType);
        return;
    }

    if (!highMipPath.empty() && Image_DumpHighMip(image, highMipPath))
        return;

    const uint32_t faceCount = image->mapType == MAPTYPE_CUBE ? 6u : 1u;
    uint32_t BaseSize =
        image::xenos_texture::CalculateBaseSize(image->texture.basemap, image->width, image->height, faceCount);

    auto format = image->texture.basemap->Format.DataFormat;
    uint32_t caps2 = 0u;
    if (image->mapType == mp::MAPTYPE_CUBE)
    {
        caps2 = image::DDSCAPS2_CUBEMAP | image::DDSCAPS2_CUBEMAP_POSITIVEX | image::DDSCAPS2_CUBEMAP_NEGATIVEX |
                image::DDSCAPS2_CUBEMAP_POSITIVEY | image::DDSCAPS2_CUBEMAP_NEGATIVEY |
                image::DDSCAPS2_CUBEMAP_POSITIVEZ | image::DDSCAPS2_CUBEMAP_NEGATIVEZ;
    }

    image::DDS_HEADER header;
    if (!image::CreateDdsHeader(header, image->width, image->height, image->depth,
                                image::xenos_texture::GetTextureLevelCount(image->texture.basemap), BaseSize,
                                image::DDSCAPS_TEXTURE | image::DDSCAPS_MIPMAP, caps2, format))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Skipping image '%s': unsupported texture format %d\n", image->name, format);
        return;
    }

    const std::string filename = GetImageDumpPath(image->name);

    std::ofstream file(filename.c_str(), std::ios::binary);
    if (!file)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Could not create DDS for image '%s': %s\n", image->name, filename.c_str());
        return;
    }

    if (image->mapType == MAPTYPE_CUBE)
    {
        image::WriteDdsHeader(file, header);

        const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(image->texture.basemap->Format.DataFormat);
        const uint32_t linearFaceSize =
            image::xenos_texture::CalculateLinearLevelSize(image->width, image->height, 0u, format);
        const uint32_t tiledFaceSize = image::xenos_texture::CalculateTiledLevelSize(
            image->width, image->height, 0u, format, image->texture.basemap->Format.Pitch);
        const uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(image->width, 0u, format);
        if (linearFaceSize == 0 || tiledFaceSize == 0 || rowPitch == 0)
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Skipping cubemap '%s': unsupported texture format %d\n", image->name,
                           format);
            return;
        }

        // TODO: handle mip levels per face for cubemaps
        for (int i = 0; i < 6; i++)
        {
            const size_t faceOffset = static_cast<size_t>(i) * tiledFaceSize;
            if (faceOffset + tiledFaceSize > image->baseSize)
            {
                Com_PrintError(CON_CHANNEL_ERROR, "Skipping cubemap '%s': pixel data is too small (have %u, need %u)\n",
                               image->name, image->baseSize,
                               static_cast<unsigned int>(faceOffset + tiledFaceSize));
                return;
            }

            unsigned char *face_pixels = image->pixels + faceOffset; // Offset for each face

            std::vector<uint8_t> swappedFace(face_pixels, face_pixels + tiledFaceSize);
            image::xenos_texture::ApplyGpuEndian(swappedFace.data(), swappedFace.size(),
                                                 static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

            // Create buffer for linear texture data
            std::vector<uint8_t> linearFace(linearFaceSize);

            if (!image::xenos_texture::UntileTextureLevel(image->width, image->height, 0, static_cast<uint32_t>(format),
                                                          image->texture.basemap->Format.Pitch, linearFace.data(),
                                                          linearFace.size(), rowPitch, swappedFace.data(),
                                                          swappedFace.size()))
            {
                Com_PrintError(CON_CHANNEL_ERROR, "Could not decode cubemap '%s' face %d\n", image->name, i);
                return;
            }

            file.write(reinterpret_cast<const char *>(linearFace.data()), linearFace.size());
        }

        file.close();
        Com_Printf(CON_CHANNEL_CONSOLEONLY, "Dumped image '%s'\n", image->name);
    }
    else if (image->mapType == MAPTYPE_2D)
    {
        // TODO: write mip levels
        image::WriteDdsHeader(file, header);

        auto format = image->texture.basemap->Format.DataFormat;
        const uint32_t linearLevelSize =
            image::xenos_texture::CalculateLinearLevelSize(image->width, image->height, 0u, format);
        const uint32_t tiledLevelSize = image::xenos_texture::CalculateTiledLevelSize(
            image->width, image->height, 0u, format, image->texture.basemap->Format.Pitch);
        const uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(image->width, 0u, format);
        if (linearLevelSize == 0 || tiledLevelSize == 0 || rowPitch == 0)
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Skipping image '%s': unsupported texture format %d\n", image->name,
                           format);
            return;
        }

        if (image->baseSize < tiledLevelSize)
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Skipping image '%s': pixel data is too small (have %u, need %u)\n",
                           image->name, image->baseSize, tiledLevelSize);
            return;
        }

        std::vector<uint8_t> pixelData(image->pixels, image->pixels + tiledLevelSize);

        image::xenos_texture::ApplyGpuEndian(pixelData.data(), pixelData.size(),
                                             static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

        // Create a linear data buffer to hold the untiled texture
        std::vector<uint8_t> linearData(linearLevelSize);

        if (!image::xenos_texture::UntileTextureLevel(image->width, image->height, 0, static_cast<uint32_t>(format),
                                                      image->texture.basemap->Format.Pitch, linearData.data(),
                                                      linearData.size(), rowPitch, pixelData.data(), pixelData.size()))
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Could not decode image '%s'\n", image->name);
            return;
        }

        file.write(reinterpret_cast<const char *>(linearData.data()), linearData.size());

        file.close();
        Com_Printf(CON_CHANNEL_CONSOLEONLY, "Dumped image '%s'\n", image->name);
    }
    else
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Skipping image '%s': unsupported map type %d\n", image->name,
                       image->mapType);
        return;
    }
}

void Cmd_imagedump()
{
    ImageList imageList;
    R_GetImageList(&imageList);

    // images bundled in xex
    // auto g_imageProgs = reinterpret_cast<GfxImage *>(0x84FEA6D0);
    // for (unsigned int i = 0; i < 10; i++)
    // {
    //     imageList.image[imageList.count++] = &g_imageProgs[i];
    // }

    CreateDirectoryA(DUMP_DIR, 0);
    CreateDirectoryA((std::string(DUMP_DIR) + "\\images").c_str(), 0);

    const std::map<std::string, std::string> highMipFiles = CollectHighMipFiles();
    for (unsigned int i = 0; i < imageList.count; i++)
    {
        auto image = imageList.image[i];
        if (image == NULL)
            continue;

        Image_Dump(image, FindHighMipPathForImage(highMipFiles, image->name));
    }
}

bool Image_Replace_2D(GfxImage *image, const image::DdsImage &ddsImage, uint32_t ddsFirstMipLevel)
{
    if (image->mapType != MAPTYPE_2D)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' is not a 2D map!\n", image->name);
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
        Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' replacement DDS has no mip %u: mipCount=%u\n", image->name,
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
            Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' has unsupported replacement format %d!\n", image->name,
                           format);
        }
        else if (ddsImage.data.size() < requiredDDSSize)
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' DDS data is too small: have=%u need=%u for %u mip levels\n",
                           image->name, static_cast<unsigned int>(ddsImage.data.size()),
                           static_cast<unsigned int>(requiredDDSSize), nonPackedLevelCount);
        }
        else
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' replacement exceeds texture memory: have=%u need=%u\n",
                           image->name, static_cast<unsigned int>(image->cardMemory.platform[0]),
                           static_cast<unsigned int>(requiredTextureBytes));
        }

        return false;
    }

    if (baseData == NULL || mipData == NULL)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image_Replace_2D: Image '%s' has no valid texture memory!\n", image->name);
        return false;
    }

    uint32_t ddsOffset =
        image::CalculateDdsMipOffset(ddsImage.header.dwWidth, ddsImage.header.dwHeight, format, ddsFirstMipLevel);

    for (uint32_t localMipLevel = 0; localMipLevel < nonPackedLevelCount; localMipLevel++)
    {
        const uint32_t ddsMipLevel = ddsFirstMipLevel + localMipLevel;
        uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(ddsImage.header.dwWidth, ddsMipLevel, format);
        uint32_t ddsMipLevelSize = image::xenos_texture::CalculateLinearLevelSize(
            ddsImage.header.dwWidth, ddsImage.header.dwHeight, ddsMipLevel, format);
        uint32_t tiledMipLevelSize = image::xenos_texture::CalculateTiledLevelSize(
            image->width, image->height, localMipLevel, format, image->texture.basemap->Format.Pitch);

        if (ddsMipLevelSize == 0 || tiledMipLevelSize == 0 || rowPitch == 0)
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Image_Replace_2D: Unsupported format %d for image '%s' mip level %u\n",
                           image->texture.basemap->Format.DataFormat, image->name, localMipLevel);
            return false;
        }

        // Ensure we're not reading out of bounds
        if (ddsOffset + ddsMipLevelSize > ddsImage.data.size())
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Image_Replace_2D: Image '%s' mip level %u exceeds DDS data size\n",
                           image->name, ddsMipLevel);
            return false;
        }

        std::vector<uint8_t> levelData(ddsImage.data.begin() + ddsOffset,
                                       ddsImage.data.begin() + ddsOffset + ddsMipLevelSize);

        image::xenos_texture::ApplyGpuEndian(levelData.data(), levelData.size(),
                                             static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

        unsigned char *destination = baseData;
        if (localMipLevel > 0)
        {
            destination = mipData + image::xenos_texture::CalculateMipLevelOffset(image->width, image->height,
                                                                                  localMipLevel, format, 1u);
        }

        std::vector<uint8_t> tiledData(tiledMipLevelSize);

        if (!image::xenos_texture::TileTextureLevel(image->width, image->height, localMipLevel, format,
                                                    image->texture.basemap->Format.Pitch, tiledData.data(),
                                                    tiledData.size(), levelData.data(), levelData.size(), rowPitch))
        {
            Com_PrintError(
                CON_CHANNEL_ERROR,
                "Image_Replace_2D: Failed to tile mip level %d for image '%s' rowPitch=%u sourceSize=%u destSize=%u\n",
                localMipLevel, image->name, rowPitch, static_cast<unsigned int>(levelData.size()), tiledMipLevelSize);
            return false;
        }

        memcpy(destination, tiledData.data(), tiledMipLevelSize);

        ddsOffset += ddsMipLevelSize;
    }

    return true;
}

bool Image_Replace_Cube(GfxImage *image, const image::DdsImage &ddsImage)
{
    if (image->mapType != MAPTYPE_CUBE)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' is not a cube map!\n", image->name);
        return false;
    }

    const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(image->texture.basemap->Format.DataFormat);

    unsigned int face_size = image::xenos_texture::CalculateLinearLevelSize(image->width, image->height, 0, format);
    unsigned int rowPitch = image::xenos_texture::CalculateLinearRowPitch(image->width, 0, format);
    unsigned int tiledFaceSize = image::xenos_texture::CalculateTiledLevelSize(image->width, image->height, 0, format,
                                                                               image->texture.basemap->Format.Pitch);
    unsigned int tiledBaseSize =
        image::xenos_texture::CalculateBaseSize(image->texture.basemap, image->width, image->height, 6u);
    unsigned char *baseData = image::xenos_texture::GetTextureBase(image->texture.basemap, image->pixels);

    if (face_size == 0 || rowPitch == 0 || tiledFaceSize == 0 || tiledBaseSize < tiledFaceSize * 6u)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' has unsupported format %d!\n", image->name, format);
        return false;
    }

    if (baseData == NULL)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image_Replace_Cube: Image '%s' has no valid texture memory!\n", image->name);
        return false;
    }

    size_t requiredDDSSize = 0;
    if (!ValidateCubeReplacementData(image, ddsImage, format, face_size, tiledBaseSize, &requiredDDSSize))
    {
        if (requiredDDSSize == 0)
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' has unsupported cube replacement format %d!\n", image->name,
                           format);
        }
        else if (ddsImage.data.size() < requiredDDSSize)
        {
            Com_PrintError(CON_CHANNEL_ERROR,
                           "Image_Replace_Cube: Image '%s' DDS is too small for 6 faces: have=%u need=%u\n",
                           image->name, static_cast<unsigned int>(ddsImage.data.size()),
                           static_cast<unsigned int>(requiredDDSSize));
        }
        else
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' cube replacement exceeds texture memory: have=%u need=%u\n",
                           image->name, static_cast<unsigned int>(image->cardMemory.platform[0]), tiledBaseSize);
        }

        return false;
    }

    for (int i = 0; i < 6; i++)
    {
        const unsigned char *face_pixels = ddsImage.data.data() + (i * face_size);
        unsigned char *face_destination = baseData + (i * tiledFaceSize);

        // Create a buffer for the tiled texture data
        std::vector<uint8_t> tiledData(tiledFaceSize);

        if (!image::xenos_texture::TileTextureLevel(image->width, image->height, 0, static_cast<uint32_t>(format),
                                                    image->texture.basemap->Format.Pitch, tiledData.data(),
                                                    tiledData.size(), face_pixels, face_size, rowPitch))
        {
            Com_PrintError(
                CON_CHANNEL_ERROR,
                "Image_Replace_Cube: Failed to tile image '%s' face %d rowPitch=%u sourceSize=%u destSize=%u\n",
                image->name, i, rowPitch, face_size, tiledFaceSize);
            return false;
        }

        image::xenos_texture::ApplyGpuEndian(tiledData.data(), tiledData.size(),
                                             static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

        // Copy the data to the image
        memcpy(face_destination, tiledData.data(), tiledFaceSize);
    }

    return true;
}

void Image_Replace(GfxImage *image)
{
    const std::string replacement_base_dir = Config::GetModBasePath() + "\\images";
    const std::string replacement_path = replacement_base_dir + "\\" + image->name + ".dds";

    if (!filesystem::file_exists(replacement_path))
    {
        return;
    }

    image::DdsImage ddsImage = image::LoadDdsFromFile(replacement_path.c_str());
    if (ddsImage.data.empty())
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Failed to load DDS file: %s\n", replacement_path.c_str());
        return;
    }

    if (ddsImage.header.dwSize != image::DDS_HEADER_SIZE ||
        ddsImage.header.ddspf.dwSize != image::DDS_PIXEL_FORMAT_SIZE)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' has an invalid DDS header: size=%u pixelFormatSize=%u\n",
                       image->name, ddsImage.header.dwSize, ddsImage.header.ddspf.dwSize);
        return;
    }

    GPUTEXTUREFORMAT ddsFormat;
    if (!ddsImage.GetGpuFormat(&ddsFormat))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' has an unsupported DDS format: flags=0x%X fourCC=0x%X\n",
                       image->name, ddsImage.header.ddspf.dwFlags, ddsImage.header.ddspf.dwFourCC);
        return;
    }

    if (static_cast<uint32_t>(image->texture.basemap->Format.DataFormat) != static_cast<uint32_t>(ddsFormat))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' format does not match DDS file: Expected %d, Got %d\n",
                       image->name, static_cast<uint32_t>(image->texture.basemap->Format.DataFormat),
                       static_cast<uint32_t>(ddsFormat));
        return;
    }

    const bool ddsIsCubemap = ddsImage.IsCubemap();
    const bool ddsMatchesImageDimensions =
        image->width == ddsImage.header.dwWidth && image->height == ddsImage.header.dwHeight;
    const bool ddsMatchesStreamDimensions = image->streaming && image->mapType == MAPTYPE_2D && !ddsIsCubemap &&
                                            ddsImage.header.dwWidth == static_cast<uint32_t>(image->width) * 2u &&
                                            ddsImage.header.dwHeight == static_cast<uint32_t>(image->height) * 2u;
    uint32_t ddsFirstMipLevel = 0;

    if (image->mapType == MAPTYPE_2D && ddsIsCubemap)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' is 2D but replacement DDS is a cubemap!\n", image->name);
        return;
    }

    if (image->streaming && image->mapType == MAPTYPE_2D)
    {
        if (!ddsMatchesStreamDimensions)
        {
            Com_PrintError(CON_CHANNEL_ERROR,
                           "Streamed image '%s' replacement must include the streamed mip: expected=%ux%u got=%ux%u "
                           "%s\n",
                           image->name, static_cast<uint32_t>(image->width) * 2u,
                           static_cast<uint32_t>(image->height) * 2u, ddsImage.header.dwWidth, ddsImage.header.dwHeight,
                           replacement_path.c_str());
            return;
        }

        const uint32_t ddsMipCount = ddsImage.GetMipCount();
        if (ddsMipCount < 2u)
        {
            Com_PrintError(CON_CHANNEL_ERROR,
                           "Image '%s' replacement DDS starts at the streamed mip but has no resident mip: "
                           "%ux%u mipCount=%u\n",
                           image->name, ddsImage.header.dwWidth, ddsImage.header.dwHeight, ddsMipCount);
            return;
        }

        ddsFirstMipLevel = 1u;
    }
    else if (!ddsMatchesImageDimensions)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' dimensions do not match DDS file: image=%ux%u dds=%ux%u %s\n",
                       image->name, image->width, image->height, ddsImage.header.dwWidth, ddsImage.header.dwHeight,
                       replacement_path.c_str());
        return;
    }

    if (image->mapType == MAPTYPE_CUBE && !ddsIsCubemap)
    {
        const uint32_t faceSize =
            image::xenos_texture::CalculateLinearLevelSize(image->width, image->height, 0, ddsFormat);
        if (faceSize == 0 || ddsImage.data.size() < static_cast<size_t>(faceSize) * 6u)
        {
            Com_PrintError(CON_CHANNEL_ERROR,
                           "Image '%s' is a cubemap but replacement DDS is not a valid 6-face cubemap!\n", image->name);
            return;
        }

        Com_Printf(CON_CHANNEL_CONSOLEONLY,
                   "Image '%s' replacement DDS has no cubemap caps but contains enough data for 6 sequential faces; "
                   "accepting it.\n",
                   image->name);
    }

    bool replaced = false;
    if (image->mapType == MAPTYPE_2D)
    {
        replaced = Image_Replace_2D(image, ddsImage, ddsFirstMipLevel);
    }
    else if (image->mapType == MAPTYPE_CUBE)
    {
        replaced = Image_Replace_Cube(image, ddsImage);
    }
    else
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' is not a 2D or cube map!\n", image->name);
        return;
    }

    if (replaced)
    {
        if (ddsFirstMipLevel > 0)
        {
            g_streamedImageReplacements.insert(image->name);
        }

        Com_Printf(CON_CHANNEL_CONSOLEONLY, "Replaced image '%s'\n", image->name);
    }
}

void Load_images()
{
    g_streamedImageReplacements.clear();

    const int MAX_IMAGES = 2048;
    XAssetHeader assets[MAX_IMAGES];
    const auto count = DB_GetAllXAssetOfType_FastFile(ASSET_TYPE_IMAGE, assets, MAX_IMAGES);
    for (int i = 0; i < count; i++)
    {
        GfxImage *image = assets[i].image;
        Image_Replace(image);
    }
}

Detour CG_RegisterGraphics_Detour;

void CG_RegisterGraphics_Hook(int localClientNum, const char *mapname)
{
    CG_RegisterGraphics_Detour.GetOriginal<decltype(CG_RegisterGraphics)>()(localClientNum, mapname);
    Load_images();
}

bool R_StreamLoadHighMipReplacement(const char *filename, unsigned int bytesToRead, unsigned __int8 *outData)
{
    std::string asset_name = extract_filename(filename);
    auto asset = DB_FindXAssetEntry(ASSET_TYPE_IMAGE, asset_name.c_str());

    if (!asset)
    {
        return false;
    }

    auto image = asset->entry.asset.header.image;

    if (image == NULL || image->texture.basemap == NULL || image->mapType != MAPTYPE_2D)
    {
        return false;
    }

    const uint32_t highMipWidth = static_cast<uint32_t>(image->width) * 2u;
    const uint32_t highMipHeight = static_cast<uint32_t>(image->height) * 2u;
    const bool blockStockStream = g_streamedImageReplacements.find(asset_name) != g_streamedImageReplacements.end();

    if (!blockStockStream)
    {
        return false;
    }

    const auto tryReplaceHighMipDDS = [&](const std::string &replacement_path, bool quietDimensionMismatch) -> bool
    {
        image::DdsImage ddsImage = image::LoadDdsFromFile(replacement_path.c_str());
        if (ddsImage.data.empty())
        {
            Com_PrintError(CON_CHANNEL_ERROR, "R_StreamLoadHighMipReplacement: Failed to load DDS file: %s\n",
                           replacement_path.c_str());
            return false;
        }

        if (ddsImage.header.dwSize != image::DDS_HEADER_SIZE ||
            ddsImage.header.ddspf.dwSize != image::DDS_PIXEL_FORMAT_SIZE)
        {
            Com_PrintError(CON_CHANNEL_ERROR,
                           "R_StreamLoadHighMipReplacement: Image '%s' has an invalid DDS header: size=%u "
                           "pixelFormatSize=%u\n",
                           asset_name.c_str(), ddsImage.header.dwSize, ddsImage.header.ddspf.dwSize);
            return false;
        }

        GPUTEXTUREFORMAT ddsFormat;
        if (!ddsImage.GetGpuFormat(&ddsFormat))
        {
            Com_PrintError(CON_CHANNEL_ERROR,
                           "R_StreamLoadHighMipReplacement: Image '%s' has an unsupported DDS format: flags=0x%X "
                           "fourCC=0x%X\n",
                           asset_name.c_str(), ddsImage.header.ddspf.dwFlags, ddsImage.header.ddspf.dwFourCC);
            return false;
        }

        if (static_cast<uint32_t>(image->texture.basemap->Format.DataFormat) != static_cast<uint32_t>(ddsFormat))
        {
            Com_PrintError(CON_CHANNEL_ERROR,
                           "R_StreamLoadHighMipReplacement: Image '%s' format does not match DDS: expected=%u "
                           "got=%u\n",
                           asset_name.c_str(), static_cast<uint32_t>(image->texture.basemap->Format.DataFormat),
                           static_cast<uint32_t>(ddsFormat));
            return false;
        }

        if (ddsImage.header.dwWidth != highMipWidth || ddsImage.header.dwHeight != highMipHeight)
        {
            if (!quietDimensionMismatch)
            {
                Com_PrintError(CON_CHANNEL_ERROR,
                               "R_StreamLoadHighMipReplacement: Image '%s' dimensions do not match streamed mip: "
                               "expected=%ux%u got=%ux%u\n",
                               asset_name.c_str(), highMipWidth, highMipHeight, ddsImage.header.dwWidth,
                               ddsImage.header.dwHeight);
            }

            return false;
        }

        const uint32_t ddsMipCount = ddsImage.GetMipCount();
        if (ddsMipCount < 2u)
        {
            Com_PrintError(CON_CHANNEL_ERROR,
                           "R_StreamLoadHighMipReplacement: Image '%s' replacement DDS must include stream and "
                           "resident mips: mipCount=%u\n",
                           asset_name.c_str(), ddsMipCount);
            return false;
        }

        const uint32_t sourceSize = image::xenos_texture::CalculateLinearLevelSize(
            ddsImage.header.dwWidth, ddsImage.header.dwHeight, 0u, ddsFormat);
        const uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(ddsImage.header.dwWidth, 0u, ddsFormat);
        const uint32_t tiledSize = image::xenos_texture::CalculateTiledLevelSize(
            ddsImage.header.dwWidth, ddsImage.header.dwHeight, 0u, ddsFormat, 0u);

        if (sourceSize == 0 || rowPitch == 0 || tiledSize == 0)
        {
            Com_PrintError(CON_CHANNEL_ERROR,
                           "R_StreamLoadHighMipReplacement: Image '%s' has unsupported replacement format %u\n",
                           asset_name.c_str(), static_cast<uint32_t>(ddsFormat));
            return false;
        }

        if (ddsImage.data.size() < sourceSize)
        {
            Com_PrintError(CON_CHANNEL_ERROR,
                           "R_StreamLoadHighMipReplacement: Image '%s' DDS data is too small: have=%u need=%u\n",
                           asset_name.c_str(), static_cast<unsigned int>(ddsImage.data.size()), sourceSize);
            return false;
        }

        if (tiledSize != bytesToRead)
        {
            Com_PrintError(CON_CHANNEL_ERROR,
                           "R_StreamLoadHighMipReplacement: Image '%s' stream size mismatch: expected=%u got=%u\n",
                           asset_name.c_str(), bytesToRead, tiledSize);
            return false;
        }

        std::vector<uint8_t> buffer(ddsImage.data.begin(), ddsImage.data.begin() + sourceSize);
        image::xenos_texture::ApplyGpuEndian(buffer.data(), buffer.size(),
                                             static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

        if (!image::xenos_texture::TileTextureLevel(ddsImage.header.dwWidth, ddsImage.header.dwHeight, 0u, ddsFormat,
                                                    0u, outData, bytesToRead, buffer.data(), buffer.size(), rowPitch))
        {
            Com_PrintError(CON_CHANNEL_ERROR, "R_StreamLoadHighMipReplacement: Failed to tile image '%s'\n",
                           asset_name.c_str());
            return false;
        }

        Com_Printf(CON_CHANNEL_CONSOLEONLY, "Replaced streamed image '%s'\n", asset_name.c_str());
        return true;
    };

    const std::string combined_path = Config::GetModBasePath() + "\\images" + "\\" + asset_name + ".dds";
    if (filesystem::file_exists(combined_path) && tryReplaceHighMipDDS(combined_path, true))
    {
        return true;
    }

    Com_PrintError(CON_CHANNEL_ERROR, "R_StreamLoadHighMipReplacement: Blocking stock stream for replaced image '%s'\n",
                   asset_name.c_str());
    if (outData != NULL && bytesToRead > 0)
    {
        memset(outData, 0, bytesToRead);
    }

    return true;
}

Detour R_StreamLoadFileSynchronously_Detour;

int R_StreamLoadFileSynchronously_Hook(const char *filename, unsigned int bytesToRead, unsigned __int8 *outData)
{
    if (R_StreamLoadHighMipReplacement(filename, bytesToRead, outData))
    {
        return 1;
    }

    // Let the game handle stock streams only when we did not accept a streamed replacement for this image.
    return R_StreamLoadFileSynchronously_Detour.GetOriginal<decltype(R_StreamLoadFileSynchronously)>()(
        filename, bytesToRead, outData);
}

image_loader::image_loader()
{
    // Load raw texture replacements from active mod folder
    CG_RegisterGraphics_Detour = Detour(CG_RegisterGraphics, CG_RegisterGraphics_Hook);
    CG_RegisterGraphics_Detour.Install();

    // Load streamed mip texture replacements from active mod folder
    R_StreamLoadFileSynchronously_Detour = Detour(R_StreamLoadFileSynchronously, R_StreamLoadFileSynchronously_Hook);
    R_StreamLoadFileSynchronously_Detour.Install();

    command::add("imagedump", Cmd_imagedump);
}

image_loader::~image_loader()
{
    CG_RegisterGraphics_Detour.Remove();

    R_StreamLoadFileSynchronously_Detour.Remove();
}
} // namespace mp
} // namespace iw3
