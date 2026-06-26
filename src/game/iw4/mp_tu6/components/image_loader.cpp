#include "pch.h"
#include "common/config.h"
#include "events.h"
#include "image_loader.h"
#include "xenos_texture.h"

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD) - 1)
#endif

namespace
{
namespace game = iw4::mp_tu6;

const uint32_t DDS_MAGIC = MAKEFOURCC('D', 'D', 'S', ' ');
const uint32_t DDS_HEADER_SIZE = 124;
const uint32_t DDS_PIXEL_FORMAT_SIZE = 32;
const uint32_t DDPF_ALPHAPIXELS = 0x1;
const uint32_t DDPF_FOURCC = 0x4;
const uint32_t DDPF_RGB = 0x40;
const uint32_t DDPF_LUMINANCE = 0x20000;
const uint32_t DXT1_FOURCC = MAKEFOURCC('D', 'X', 'T', '1');
const uint32_t DXT3_FOURCC = MAKEFOURCC('D', 'X', 'T', '3');
const uint32_t DXT5_FOURCC = MAKEFOURCC('D', 'X', 'T', '5');
const uint32_t DXN_FOURCC = MAKEFOURCC('A', 'T', 'I', '2');
const uint32_t DDSCAPS2_CUBEMAP = 0x200;
const uint32_t DDSCAPS2_CUBEMAP_POSITIVEX = 0x400;
const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800;
const uint32_t DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000;
const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000;
const uint32_t DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000;
const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000;
const uint32_t DDSCAPS2_CUBEMAP_ALL_FACES = DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_NEGATIVEX |
                                            DDSCAPS2_CUBEMAP_POSITIVEY | DDSCAPS2_CUBEMAP_NEGATIVEY |
                                            DDSCAPS2_CUBEMAP_POSITIVEZ | DDSCAPS2_CUBEMAP_NEGATIVEZ;
const uint32_t IW4_STREAM_PIXEL_SIZE_MASK = 0x3FFFFFF;
const uint32_t DEBUG_LINKED_IMAGE_LIMIT = 128;
const uint32_t DEBUG_STREAM_IMAGE_LIMIT = 128;
const uint32_t DEBUG_REPLACEMENT_FILE_LIMIT = 64;

uint32_t g_debugLinkedImageCount = 0;
uint32_t g_debugStreamImageCount = 0;

struct DDSHeader
{
    uint32_t magic;
    uint32_t size;
    uint32_t flags;
    uint32_t height;
    uint32_t width;
    uint32_t pitchOrLinearSize;
    uint32_t depth;
    uint32_t mipMapCount;
    uint32_t reserved1[11];
    struct
    {
        uint32_t size;
        uint32_t flags;
        uint32_t fourCC;
        uint32_t rgbBitCount;
        uint32_t rBitMask;
        uint32_t gBitMask;
        uint32_t bBitMask;
        uint32_t aBitMask;
    } pixelFormat;
    uint32_t caps;
    uint32_t caps2;
    uint32_t caps3;
    uint32_t caps4;
    uint32_t reserved2;
};
static_assert(sizeof(DDSHeader) == 128, "");

struct DDSImage
{
    DDSHeader header;
    std::vector<uint8_t> data;
};

void DebugLog(const char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    _vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    buffer[sizeof(buffer) - 1] = '\0';

    DbgPrint("[codxe][IW4 TU6] image_loader: debug: %s", buffer);
}

void PrintImageError(const char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    _vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    buffer[sizeof(buffer) - 1] = '\0';

    DbgPrint("[codxe][IW4 TU6] image_loader: %s", buffer);
    game::Com_Printf(0, "[codxe][IW4 TU6] image_loader: %s", buffer);
}

void PrintImageInfo(const char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    _vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    buffer[sizeof(buffer) - 1] = '\0';

    DbgPrint("[codxe][IW4 TU6] image_loader: %s", buffer);
    game::Com_Printf(0, "[codxe][IW4 TU6] image_loader: %s", buffer);
}

bool ImageFileExists(const std::string &path)
{
    const DWORD attributes = GetFileAttributesA(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

std::string GetReplacementDirectory()
{
    return Config::GetModBasePath() + "\\images";
}

void DebugListReplacementFiles()
{
    const std::string replacementDirectory = GetReplacementDirectory();
    DebugLog("replacement directory='%s'\n", replacementDirectory.c_str());

    const DWORD attributes = GetFileAttributesA(replacementDirectory.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        DebugLog("replacement directory missing/inaccessible error=0x%08X\n", GetLastError());
        return;
    }

    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        DebugLog("replacement path exists but is not a directory\n");
        return;
    }

    WIN32_FIND_DATAA findData;
    const std::string searchPath = replacementDirectory + "\\*.dds";
    HANDLE findHandle = FindFirstFileA(searchPath.c_str(), &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        DebugLog("no replacement DDS files matched '%s' error=0x%08X\n", searchPath.c_str(), GetLastError());
        return;
    }

    uint32_t replacementFileCount = 0;
    do
    {
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            if (replacementFileCount < DEBUG_REPLACEMENT_FILE_LIMIT)
                DebugLog("replacement file[%u]='%s'\n", replacementFileCount, findData.cFileName);
            replacementFileCount++;
        }
    } while (FindNextFileA(findHandle, &findData) != 0);

    FindClose(findHandle);
    DebugLog("replacement file count=%u\n", replacementFileCount);
}

void SwapDDSHeaderEndian(DDSHeader &header)
{
    header.magic = _byteswap_ulong(header.magic);
    header.size = _byteswap_ulong(header.size);
    header.flags = _byteswap_ulong(header.flags);
    header.height = _byteswap_ulong(header.height);
    header.width = _byteswap_ulong(header.width);
    header.pitchOrLinearSize = _byteswap_ulong(header.pitchOrLinearSize);
    header.depth = _byteswap_ulong(header.depth);
    header.mipMapCount = _byteswap_ulong(header.mipMapCount);

    for (int i = 0; i < 11; i++)
        header.reserved1[i] = _byteswap_ulong(header.reserved1[i]);

    header.pixelFormat.size = _byteswap_ulong(header.pixelFormat.size);
    header.pixelFormat.flags = _byteswap_ulong(header.pixelFormat.flags);
    header.pixelFormat.fourCC = _byteswap_ulong(header.pixelFormat.fourCC);
    header.pixelFormat.rgbBitCount = _byteswap_ulong(header.pixelFormat.rgbBitCount);
    header.pixelFormat.rBitMask = _byteswap_ulong(header.pixelFormat.rBitMask);
    header.pixelFormat.gBitMask = _byteswap_ulong(header.pixelFormat.gBitMask);
    header.pixelFormat.bBitMask = _byteswap_ulong(header.pixelFormat.bBitMask);
    header.pixelFormat.aBitMask = _byteswap_ulong(header.pixelFormat.aBitMask);

    header.caps = _byteswap_ulong(header.caps);
    header.caps2 = _byteswap_ulong(header.caps2);
    header.caps3 = _byteswap_ulong(header.caps3);
    header.caps4 = _byteswap_ulong(header.caps4);
    header.reserved2 = _byteswap_ulong(header.reserved2);
}

DDSImage ReadDDSFile(const std::string &filepath)
{
    DDSImage ddsImage;
    std::ifstream file(filepath.c_str(), std::ios::binary);

    if (!file.is_open())
        return ddsImage;

    file.read(reinterpret_cast<char *>(&ddsImage.header), sizeof(DDSHeader));
    if (!file || file.gcount() != sizeof(DDSHeader))
        return DDSImage();

    if (_byteswap_ulong(ddsImage.header.magic) != DDS_MAGIC)
        return DDSImage();

    SwapDDSHeaderEndian(ddsImage.header);

    file.seekg(0, std::ios::end);
    const std::streampos fileSize = file.tellg();
    if (fileSize == std::streampos(-1))
        return DDSImage();

    file.seekg(sizeof(DDSHeader), std::ios::beg);
    const size_t dataSize = static_cast<size_t>(fileSize) - sizeof(DDSHeader);
    ddsImage.data.resize(dataSize);

    if (dataSize > 0)
    {
        file.read(reinterpret_cast<char *>(&ddsImage.data[0]), dataSize);
        if (!file || static_cast<size_t>(file.gcount()) != dataSize)
            return DDSImage();
    }

    return ddsImage;
}

std::string GetReplacementPath(const char *imageName)
{
    return GetReplacementDirectory() + "\\" + imageName + ".dds";
}

bool DDSIsCubemap(const DDSImage &ddsImage)
{
    return (ddsImage.header.caps2 & DDSCAPS2_CUBEMAP) != 0 ||
           (ddsImage.header.caps2 & DDSCAPS2_CUBEMAP_ALL_FACES) == DDSCAPS2_CUBEMAP_ALL_FACES;
}

uint32_t GetDDSMipCount(const DDSImage &ddsImage)
{
    return max(1u, ddsImage.header.mipMapCount);
}

uint32_t GetMipDimension(uint32_t dimension, uint32_t mipLevel)
{
    return max(1u, dimension >> mipLevel);
}

bool GetDDSFormat(const DDSImage &ddsImage, GPUTEXTUREFORMAT *format)
{
    const DDSHeader &header = ddsImage.header;

    if ((header.pixelFormat.flags & DDPF_FOURCC) != 0)
    {
        switch (header.pixelFormat.fourCC)
        {
        case DXT1_FOURCC:
            *format = GPUTEXTUREFORMAT_DXT1;
            return true;
        case DXT3_FOURCC:
            *format = GPUTEXTUREFORMAT_DXT2_3;
            return true;
        case DXT5_FOURCC:
            *format = GPUTEXTUREFORMAT_DXT4_5;
            return true;
        case DXN_FOURCC:
            *format = GPUTEXTUREFORMAT_DXN;
            return true;
        default:
            return false;
        }
    }

    if ((header.pixelFormat.flags & DDPF_LUMINANCE) != 0)
    {
        if (header.pixelFormat.rgbBitCount == 8 && header.pixelFormat.rBitMask == 0x000000FF)
        {
            *format = GPUTEXTUREFORMAT_8;
            return true;
        }

        if ((header.pixelFormat.flags & DDPF_ALPHAPIXELS) != 0 && header.pixelFormat.rgbBitCount == 16 &&
            header.pixelFormat.rBitMask == 0x000000FF && header.pixelFormat.gBitMask == 0x0000FF00)
        {
            *format = GPUTEXTUREFORMAT_8_8;
            return true;
        }
    }

    if ((header.pixelFormat.flags & DDPF_RGB) != 0 && (header.pixelFormat.flags & DDPF_ALPHAPIXELS) != 0 &&
        header.pixelFormat.rgbBitCount == 32 && header.pixelFormat.rBitMask == 0x00FF0000 &&
        header.pixelFormat.gBitMask == 0x0000FF00 && header.pixelFormat.bBitMask == 0x000000FF &&
        header.pixelFormat.aBitMask == 0xFF000000)
    {
        *format = GPUTEXTUREFORMAT_8_8_8_8;
        return true;
    }

    return false;
}

bool ValidateDDSHeader(const game::GfxImage *image, const DDSImage &ddsImage, const std::string &path,
                       GPUTEXTUREFORMAT *ddsFormat)
{
    if (ddsImage.data.empty())
    {
        PrintImageError("failed to load DDS for image '%s': %s\n", image->name, path.c_str());
        return false;
    }

    if (ddsImage.header.size != DDS_HEADER_SIZE || ddsImage.header.pixelFormat.size != DDS_PIXEL_FORMAT_SIZE)
    {
        PrintImageError("image '%s' has an invalid DDS header: size=%u pixelFormatSize=%u\n", image->name,
                        ddsImage.header.size, ddsImage.header.pixelFormat.size);
        return false;
    }

    if (!GetDDSFormat(ddsImage, ddsFormat))
    {
        PrintImageError("image '%s' has an unsupported DDS format: flags=0x%X fourCC=0x%X bitCount=%u\n", image->name,
                        ddsImage.header.pixelFormat.flags, ddsImage.header.pixelFormat.fourCC,
                        ddsImage.header.pixelFormat.rgbBitCount);
        return false;
    }

    const uint32_t imageFormat = image->texture.basemap.Format.DataFormat;
    if (imageFormat != static_cast<uint32_t>(*ddsFormat))
    {
        PrintImageError("image '%s' format does not match DDS: expected=%u got=%u\n", image->name, imageFormat,
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
        const uint32_t levelSize = xenos_texture::CalculateLinearLevelSize(width, height, mipLevel, format);
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
        const uint32_t levelSize = xenos_texture::CalculateLinearLevelSize(width, height, currentMip, format);
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
        const uint32_t levelSize = xenos_texture::CalculateTiledLevelSize(width, height, mipLevel, format, 0u);
        if (levelSize == 0)
            return 0;

        requiredSize += static_cast<size_t>(levelSize) * faceCount;
    }

    return requiredSize;
}

bool Validate2DReplacementData(const game::GfxImage *image, const DDSImage &ddsImage, GPUTEXTUREFORMAT format,
                               uint32_t replacementLevelCount, size_t *requiredDDSSize, size_t *requiredTextureBytes)
{
    *requiredDDSSize =
        CalculateRequiredLinearDataSize(image->width, image->height, format, 0u, replacementLevelCount, 1u);
    if (*requiredDDSSize == 0)
        return false;

    if (ddsImage.data.size() < *requiredDDSSize)
        return false;

    const D3DBaseTexture *texture = &image->texture.basemap;
    const uint32_t baseSize = xenos_texture::CalculateBaseSize(texture, image->width, image->height, 1u);
    const size_t mipBytes =
        CalculateRequiredMipTextureBytes(image->width, image->height, format, 1u, replacementLevelCount, 1u);

    *requiredTextureBytes = static_cast<size_t>(baseSize) + mipBytes;
    const int cardMemory = image->cardMemory.platform[0];
    if (cardMemory > 0 && *requiredTextureBytes > static_cast<size_t>(cardMemory))
        return false;

    return true;
}

bool ValidateResidentMipCount(const game::GfxImage *image, const DDSImage &ddsImage, uint32_t textureLevelCount)
{
    const uint32_t ddsMipCount = GetDDSMipCount(ddsImage);
    if (ddsMipCount != textureLevelCount)
    {
        PrintImageError("image '%s' mip count does not match DDS: image=%u dds=%u\n", image->name, textureLevelCount,
                        ddsMipCount);
        return false;
    }

    return true;
}

bool ValidateDDSDataSize(const game::GfxImage *image, const DDSImage &ddsImage, GPUTEXTUREFORMAT format,
                         uint32_t mipCount, uint32_t faceCount)
{
    const size_t expectedSize =
        CalculateRequiredLinearDataSize(ddsImage.header.width, ddsImage.header.height, format, 0u, mipCount, faceCount);
    if (expectedSize == 0)
    {
        PrintImageError("image '%s' has unsupported DDS dimensions or format\n", image->name);
        return false;
    }

    if (ddsImage.data.size() != expectedSize)
    {
        PrintImageError("image '%s' DDS data size does not match shape: have=%u expected=%u mips=%u faces=%u\n",
                        image->name, static_cast<unsigned int>(ddsImage.data.size()),
                        static_cast<unsigned int>(expectedSize), mipCount, faceCount);
        return false;
    }

    return true;
}

bool ValidateCubeReplacementData(const game::GfxImage *image, const DDSImage &ddsImage, GPUTEXTUREFORMAT format,
                                 uint32_t faceSize, uint32_t tiledBaseSize, size_t *requiredDDSSize)
{
    *requiredDDSSize = static_cast<size_t>(faceSize) * 6u;
    if (faceSize == 0 || *requiredDDSSize == 0)
        return false;

    if (ddsImage.data.size() != *requiredDDSSize)
        return false;

    const int cardMemory = image->cardMemory.platform[0];
    if (cardMemory > 0 && static_cast<size_t>(tiledBaseSize) > static_cast<size_t>(cardMemory))
        return false;

    return true;
}

bool ImageHasStreamedParts(const game::GfxImage *image)
{
    if (image->streaming)
        return true;

    for (uint32_t imagePartIndex = 0; imagePartIndex < 4u; ++imagePartIndex)
    {
        if ((image->streams[imagePartIndex].pixelSize & IW4_STREAM_PIXEL_SIZE_MASK) != 0)
            return true;
    }

    return false;
}

void DebugLogImageState(const char *source, const game::GfxImage *image, bool replacementExists,
                        const std::string &replacementPath)
{
    if (image == NULL)
        return;

    DebugLog("%s image='%s' replacement=%u path='%s' mapType=%u size=%ux%u depth=%u levels=%u streaming=%u pixels=%p "
             "card=%d streamPixels=[0x%08X,0x%08X,0x%08X,0x%08X]\n",
             source, image->name != NULL ? image->name : "<null>", replacementExists ? 1u : 0u, replacementPath.c_str(),
             static_cast<uint32_t>(image->mapType), static_cast<uint32_t>(image->width),
             static_cast<uint32_t>(image->height), static_cast<uint32_t>(image->depth),
             static_cast<uint32_t>(image->levelCount), image->streaming ? 1u : 0u, image->pixels,
             image->cardMemory.platform[0], image->streams[0].pixelSize, image->streams[1].pixelSize,
             image->streams[2].pixelSize, image->streams[3].pixelSize);
}

bool Image_Replace_2D(game::GfxImage *image, const DDSImage &ddsImage)
{
    if (image->mapType != game::MAPTYPE_2D)
    {
        PrintImageError("image '%s' is not a 2D map\n", image->name);
        return false;
    }

    const D3DBaseTexture *texture = &image->texture.basemap;
    const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(texture->Format.DataFormat);
    const uint32_t levelCount = xenos_texture::GetTextureLevelCount(texture);
    const uint32_t mipTailBaseLevel =
        texture->Format.PackedMips != 0 ? xenos_texture::GetMipTailBaseLevel(image->width, image->height) : levelCount;
    if (!ValidateResidentMipCount(image, ddsImage, levelCount))
        return false;
    if (!ValidateDDSDataSize(image, ddsImage, format, levelCount, 1u))
        return false;

    const uint32_t nonPackedLevelCount = max(1u, min(levelCount, mipTailBaseLevel));
    unsigned char *baseData = xenos_texture::GetTextureBase(texture, image->pixels);
    unsigned char *mipData =
        xenos_texture::GetTextureMipBase(texture, baseData, image->width, image->height, format, 1u);

    size_t requiredDDSSize = 0;
    size_t requiredTextureBytes = 0;
    if (!Validate2DReplacementData(image, ddsImage, format, nonPackedLevelCount, &requiredDDSSize,
                                   &requiredTextureBytes))
    {
        if (requiredDDSSize == 0)
        {
            PrintImageError("image '%s' has unsupported replacement format %u\n", image->name,
                            static_cast<uint32_t>(format));
        }
        else if (ddsImage.data.size() < requiredDDSSize)
        {
            PrintImageError("image '%s' DDS data is too small: have=%u need=%u for %u mip levels\n", image->name,
                            static_cast<unsigned int>(ddsImage.data.size()), static_cast<unsigned int>(requiredDDSSize),
                            nonPackedLevelCount);
        }
        else
        {
            PrintImageError("image '%s' replacement exceeds texture memory: have=%u need=%u\n", image->name,
                            static_cast<unsigned int>(image->cardMemory.platform[0]),
                            static_cast<unsigned int>(requiredTextureBytes));
        }

        return false;
    }

    if (baseData == NULL || mipData == NULL)
    {
        PrintImageError("image '%s' has no valid texture memory\n", image->name);
        return false;
    }

    uint32_t ddsOffset = 0;

    for (uint32_t mipLevel = 0; mipLevel < nonPackedLevelCount; ++mipLevel)
    {
        const uint32_t rowPitch = xenos_texture::CalculateLinearRowPitch(image->width, mipLevel, format);
        const uint32_t ddsMipLevelSize =
            xenos_texture::CalculateLinearLevelSize(image->width, image->height, mipLevel, format);
        const uint32_t tiledMipLevelSize = xenos_texture::CalculateTiledLevelSize(image->width, image->height, mipLevel,
                                                                                  format, texture->Format.Pitch);

        if (ddsMipLevelSize == 0 || tiledMipLevelSize == 0 || rowPitch == 0)
        {
            PrintImageError("unsupported format %u for image '%s' mip level %u\n", texture->Format.DataFormat,
                            image->name, mipLevel);
            return false;
        }

        if (static_cast<size_t>(ddsOffset) + ddsMipLevelSize > ddsImage.data.size())
        {
            PrintImageError("image '%s' mip level %u exceeds DDS data size\n", image->name, mipLevel);
            return false;
        }

        std::vector<uint8_t> levelData(ddsImage.data.begin() + ddsOffset,
                                       ddsImage.data.begin() + ddsOffset + ddsMipLevelSize);
        xenos_texture::ApplyGpuEndian(&levelData[0], levelData.size(), static_cast<GPUENDIAN>(texture->Format.Endian));

        unsigned char *destination = baseData;
        if (mipLevel > 0)
        {
            destination =
                mipData + xenos_texture::CalculateMipLevelOffset(image->width, image->height, mipLevel, format, 1u);
        }

        std::vector<uint8_t> tiledData(tiledMipLevelSize);
        if (!xenos_texture::TileTextureLevel(image->width, image->height, mipLevel, format, texture->Format.Pitch,
                                             &tiledData[0], tiledData.size(), &levelData[0], levelData.size(),
                                             rowPitch))
        {
            PrintImageError("failed to tile image '%s' mip level %u\n", image->name, mipLevel);
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
        PrintImageError("image '%s' is not a cube map\n", image->name);
        return false;
    }

    const D3DBaseTexture *texture = &image->texture.basemap;
    const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(texture->Format.DataFormat);
    const uint32_t levelCount = xenos_texture::GetTextureLevelCount(texture);
    const uint32_t faceSize = xenos_texture::CalculateLinearLevelSize(image->width, image->height, 0u, format);
    const uint32_t rowPitch = xenos_texture::CalculateLinearRowPitch(image->width, 0u, format);
    const uint32_t tiledFaceSize =
        xenos_texture::CalculateTiledLevelSize(image->width, image->height, 0u, format, texture->Format.Pitch);
    const uint32_t tiledBaseSize = xenos_texture::CalculateBaseSize(texture, image->width, image->height, 6u);
    unsigned char *baseData = xenos_texture::GetTextureBase(texture, image->pixels);

    if (faceSize == 0 || rowPitch == 0 || tiledFaceSize == 0 || tiledBaseSize < tiledFaceSize * 6u)
    {
        PrintImageError("image '%s' has unsupported cube format %u\n", image->name, static_cast<uint32_t>(format));
        return false;
    }

    if (baseData == NULL)
    {
        PrintImageError("image '%s' has no valid cube texture memory\n", image->name);
        return false;
    }

    if (levelCount != 1u || GetDDSMipCount(ddsImage) != 1u)
    {
        PrintImageError("image '%s' cube replacement must be base-level only: imageMips=%u ddsMips=%u\n", image->name,
                        levelCount, GetDDSMipCount(ddsImage));
        return false;
    }

    if (!ValidateDDSDataSize(image, ddsImage, format, 1u, 6u))
        return false;

    size_t requiredDDSSize = 0;
    if (!ValidateCubeReplacementData(image, ddsImage, format, faceSize, tiledBaseSize, &requiredDDSSize))
    {
        if (ddsImage.data.size() < requiredDDSSize)
        {
            PrintImageError("image '%s' DDS is too small for 6 cube faces: have=%u need=%u\n", image->name,
                            static_cast<unsigned int>(ddsImage.data.size()),
                            static_cast<unsigned int>(requiredDDSSize));
        }
        else
        {
            PrintImageError("image '%s' cube replacement exceeds texture memory: have=%u need=%u\n", image->name,
                            static_cast<unsigned int>(image->cardMemory.platform[0]), tiledBaseSize);
        }

        return false;
    }

    for (uint32_t faceIndex = 0; faceIndex < 6u; ++faceIndex)
    {
        const unsigned char *facePixels = &ddsImage.data[faceIndex * faceSize];
        unsigned char *faceDestination = baseData + (faceIndex * tiledFaceSize);
        std::vector<uint8_t> tiledData(tiledFaceSize);

        if (!xenos_texture::TileTextureLevel(image->width, image->height, 0u, format, texture->Format.Pitch,
                                             &tiledData[0], tiledData.size(), facePixels, faceSize, rowPitch))
        {
            PrintImageError("failed to tile cube image '%s' face %u\n", image->name, faceIndex);
            return false;
        }

        xenos_texture::ApplyGpuEndian(&tiledData[0], tiledData.size(), static_cast<GPUENDIAN>(texture->Format.Endian));
        memcpy(faceDestination, &tiledData[0], tiledFaceSize);
    }

    return true;
}

bool ValidateReplacementShape(const game::GfxImage *image, const DDSImage &ddsImage)
{
    const bool ddsIsCubemap = DDSIsCubemap(ddsImage);

    if (image->mapType == game::MAPTYPE_2D && ddsIsCubemap)
    {
        PrintImageError("image '%s' is 2D but replacement DDS is a cubemap\n", image->name);
        return false;
    }

    if (image->mapType == game::MAPTYPE_CUBE && !ddsIsCubemap)
    {
        GPUTEXTUREFORMAT ddsFormat;
        if (!GetDDSFormat(ddsImage, &ddsFormat))
            return false;

        const uint32_t faceSize =
            xenos_texture::CalculateLinearLevelSize(ddsImage.header.width, ddsImage.header.height, 0u, ddsFormat);
        if (faceSize == 0 || ddsImage.data.size() < static_cast<size_t>(faceSize) * 6u)
        {
            PrintImageError("image '%s' is a cubemap but replacement DDS is not a valid 6-face cubemap\n", image->name);
            return false;
        }
    }

    return true;
}

void Image_Replace(game::GfxImage *image)
{
    if (image == NULL || image->name == NULL)
        return;

    const std::string replacementPath = GetReplacementPath(image->name);
    const bool replacementExists = ImageFileExists(replacementPath);
    if (!replacementExists)
        return;

    DebugLogImageState("resident candidate", image, replacementExists, replacementPath);

    if (ImageHasStreamedParts(image))
    {
        PrintImageInfo("found streamed replacement for image '%s'; waiting for streamed data\n", image->name);
        return;
    }

    if (image->pixels == NULL || image->cardMemory.platform[0] <= 0)
    {
        PrintImageError("image '%s' replacement exists but resident texture memory is not available\n", image->name);
        return;
    }

    DDSImage ddsImage = ReadDDSFile(replacementPath);
    GPUTEXTUREFORMAT ddsFormat;
    if (!ValidateDDSHeader(image, ddsImage, replacementPath, &ddsFormat))
        return;

    if (image->width != ddsImage.header.width || image->height != ddsImage.header.height)
    {
        PrintImageError("image '%s' dimensions do not match DDS: image=%ux%u dds=%ux%u\n", image->name, image->width,
                        image->height, ddsImage.header.width, ddsImage.header.height);
        return;
    }

    if (!ValidateReplacementShape(image, ddsImage))
        return;

    bool replaced = false;
    if (image->mapType == game::MAPTYPE_2D)
        replaced = Image_Replace_2D(image, ddsImage);
    else if (image->mapType == game::MAPTYPE_CUBE)
        replaced = Image_Replace_Cube(image, ddsImage);
    else
        PrintImageError("image '%s' is not a 2D or cube map\n", image->name);

    if (replaced)
        PrintImageInfo("replaced image '%s' (resident)\n", image->name);
}

bool FindMipLevelForDimensions(const DDSImage &ddsImage, uint32_t width, uint32_t height, uint32_t *mipLevel)
{
    const uint32_t ddsMipCount = GetDDSMipCount(ddsImage);

    for (uint32_t currentMip = 0; currentMip < ddsMipCount; ++currentMip)
    {
        if (GetMipDimension(ddsImage.header.width, currentMip) == width &&
            GetMipDimension(ddsImage.header.height, currentMip) == height)
        {
            *mipLevel = currentMip;
            return true;
        }
    }

    return false;
}

bool ValidateStreamReplacementData(const game::GfxImage *image, const DDSImage &ddsImage, GPUTEXTUREFORMAT format,
                                   uint32_t startMipLevel, uint32_t levelCount, uint32_t basePitch,
                                   size_t *requiredDDSSize, size_t *requiredTextureBytes)
{
    *requiredDDSSize = CalculateRequiredLinearDataSize(ddsImage.header.width, ddsImage.header.height, format,
                                                       startMipLevel, levelCount, 1u);
    if (*requiredDDSSize == 0)
        return false;

    const uint32_t ddsOffset =
        CalculateDDSMipOffset(ddsImage.header.width, ddsImage.header.height, format, startMipLevel);
    if (static_cast<size_t>(ddsOffset) + *requiredDDSSize > ddsImage.data.size())
        return false;

    *requiredTextureBytes = 0;
    for (uint32_t localMipLevel = 0; localMipLevel < levelCount; ++localMipLevel)
    {
        const uint32_t levelSize =
            xenos_texture::CalculateTiledLevelSize(image->width, image->height, localMipLevel, format, basePitch);
        if (levelSize == 0)
            return false;

        *requiredTextureBytes += levelSize;
    }

    const int cardMemory = image->cardMemory.platform[0];
    if (cardMemory <= 0 || *requiredTextureBytes > static_cast<size_t>(cardMemory))
        return false;

    return true;
}

bool Image_Replace_StreamCubePart(game::GfxImage *image, const DDSImage &ddsImage, GPUTEXTUREFORMAT ddsFormat,
                                  uint32_t imagePartIndex)
{
    if (imagePartIndex != 0u)
    {
        PrintImageError("streamed cube image '%s' has unsupported part %u\n", image->name, imagePartIndex);
        return false;
    }

    if (image->width != ddsImage.header.width || image->height != ddsImage.header.height)
    {
        PrintImageError("streamed cube image '%s' dimensions do not match DDS: image=%ux%u dds=%ux%u\n", image->name,
                        image->width, image->height, ddsImage.header.width, ddsImage.header.height);
        return false;
    }

    const uint32_t ddsMipCount = GetDDSMipCount(ddsImage);
    if (image->levelCount != 1u || ddsMipCount != 1u)
    {
        PrintImageError("streamed cube image '%s' must be base-level only: imageMips=%u ddsMips=%u\n", image->name,
                        static_cast<uint32_t>(image->levelCount), ddsMipCount);
        return false;
    }

    if (!ValidateDDSDataSize(image, ddsImage, ddsFormat, 1u, 6u))
        return false;

    const uint32_t rowPitch = xenos_texture::CalculateLinearRowPitch(image->width, 0u, ddsFormat);
    const uint32_t faceSize = xenos_texture::CalculateLinearLevelSize(image->width, image->height, 0u, ddsFormat);
    const uint32_t tiledFaceSize =
        xenos_texture::CalculateTiledLevelSize(image->width, image->height, 0u, ddsFormat, 0u);
    const size_t requiredTextureBytes = static_cast<size_t>(tiledFaceSize) * 6u;

    if (rowPitch == 0 || faceSize == 0 || tiledFaceSize == 0)
    {
        PrintImageError("streamed cube image '%s' has unsupported format %u\n", image->name,
                        static_cast<uint32_t>(ddsFormat));
        return false;
    }

    if (image->cardMemory.platform[0] <= 0 || requiredTextureBytes > static_cast<size_t>(image->cardMemory.platform[0]))
    {
        PrintImageError("streamed cube image '%s' replacement exceeds texture memory: have=%u need=%u\n", image->name,
                        static_cast<unsigned int>(image->cardMemory.platform[0]),
                        static_cast<unsigned int>(requiredTextureBytes));
        return false;
    }

    for (uint32_t faceIndex = 0; faceIndex < 6u; ++faceIndex)
    {
        const unsigned char *facePixels = &ddsImage.data[faceIndex * faceSize];
        std::vector<uint8_t> tiledData(tiledFaceSize);

        if (!xenos_texture::TileTextureLevel(image->width, image->height, 0u, ddsFormat, 0u, &tiledData[0],
                                             tiledData.size(), facePixels, faceSize, rowPitch))
        {
            PrintImageError("failed to tile streamed cube image '%s' face %u\n", image->name, faceIndex);
            return false;
        }

        xenos_texture::ApplyGpuEndian(&tiledData[0], tiledData.size(),
                                      static_cast<GPUENDIAN>(image->texture.basemap.Format.Endian));
        memcpy(image->pixels + static_cast<size_t>(faceIndex) * tiledFaceSize, &tiledData[0], tiledFaceSize);
    }

    return true;
}

bool Image_Replace_StreamPart(game::GfxImage *image, const DDSImage &ddsImage, uint32_t imagePartIndex)
{
    if (image == NULL || image->name == NULL || imagePartIndex >= 4u)
        return false;

    if (image->pixels == NULL)
    {
        PrintImageError("streamed image '%s' part %u replacement exists but pixel memory is not available\n",
                        image->name, imagePartIndex);
        return false;
    }

    GPUTEXTUREFORMAT ddsFormat;
    const std::string replacementPath = GetReplacementPath(image->name);
    if (!ValidateDDSHeader(image, ddsImage, replacementPath, &ddsFormat))
        return false;

    if (image->mapType == game::MAPTYPE_CUBE)
        return Image_Replace_StreamCubePart(image, ddsImage, ddsFormat, imagePartIndex);

    if (image->mapType != game::MAPTYPE_2D)
    {
        PrintImageError("streamed image '%s' is not a supported 2D or cube map\n", image->name);
        return false;
    }

    if (DDSIsCubemap(ddsImage))
    {
        PrintImageError("streamed image '%s' is 2D but replacement DDS is a cubemap\n", image->name);
        return false;
    }

    const uint32_t ddsMipCount = GetDDSMipCount(ddsImage);
    if (!ValidateDDSDataSize(image, ddsImage, ddsFormat, ddsMipCount, 1u))
        return false;

    uint32_t startMipLevel = 0;
    if (!FindMipLevelForDimensions(ddsImage, image->width, image->height, &startMipLevel))
    {
        PrintImageError("streamed image '%s' part %u dimensions do not exist in DDS: part=%ux%u dds=%ux%u\n",
                        image->name, imagePartIndex, image->width, image->height, ddsImage.header.width,
                        ddsImage.header.height);
        return false;
    }

    const game::GfxImageStreamData &streamData = image->streams[imagePartIndex];
    uint32_t levelCount = streamData.pixelSize >> 26;
    if (levelCount == 0)
        levelCount = max(1u, static_cast<uint32_t>(image->levelCount));

    const D3DBaseTexture *texture = &image->texture.basemap;
    const uint32_t textureLevelCount = xenos_texture::GetTextureLevelCount(texture);
    levelCount = max(1u, min(levelCount, textureLevelCount));

    const uint32_t mipTailBaseLevel =
        texture->Format.PackedMips != 0 ? xenos_texture::GetMipTailBaseLevel(image->width, image->height) : levelCount;
    const uint32_t replaceLevelCount = max(1u, min(levelCount, mipTailBaseLevel));

    if (startMipLevel + replaceLevelCount > ddsMipCount)
    {
        PrintImageError("streamed image '%s' part %u needs %u DDS mips from mip %u, but DDS has %u\n", image->name,
                        imagePartIndex, replaceLevelCount, startMipLevel, ddsMipCount);
        return false;
    }

    size_t requiredDDSSize = 0;
    size_t requiredTextureBytes = 0;
    const uint32_t streamBasePitch = 0u;
    if (!ValidateStreamReplacementData(image, ddsImage, ddsFormat, startMipLevel, replaceLevelCount, streamBasePitch,
                                       &requiredDDSSize, &requiredTextureBytes))
    {
        PrintImageError("streamed image '%s' part %u replacement size is invalid: ddsNeed=%u textureNeed=%u card=%u\n",
                        image->name, imagePartIndex, static_cast<unsigned int>(requiredDDSSize),
                        static_cast<unsigned int>(requiredTextureBytes),
                        static_cast<unsigned int>(image->cardMemory.platform[0]));
        return false;
    }

    DebugLog("stream replacement image='%s' part=%u startMip=%u streamLevels=%u replaceLevels=%u packedMips=%u "
             "texturePitch=%u streamPitch=%u format=%u endian=%u\n",
             image->name, imagePartIndex, startMipLevel, levelCount, replaceLevelCount,
             static_cast<uint32_t>(texture->Format.PackedMips), static_cast<uint32_t>(texture->Format.Pitch),
             streamBasePitch, static_cast<uint32_t>(texture->Format.DataFormat),
             static_cast<uint32_t>(texture->Format.Endian));

    uint32_t ddsOffset = CalculateDDSMipOffset(ddsImage.header.width, ddsImage.header.height, ddsFormat, startMipLevel);
    size_t destinationOffset = 0;

    for (uint32_t localMipLevel = 0; localMipLevel < replaceLevelCount; ++localMipLevel)
    {
        const uint32_t globalMipLevel = startMipLevel + localMipLevel;
        const uint32_t rowPitch =
            xenos_texture::CalculateLinearRowPitch(ddsImage.header.width, globalMipLevel, ddsFormat);
        const uint32_t ddsMipLevelSize = xenos_texture::CalculateLinearLevelSize(
            ddsImage.header.width, ddsImage.header.height, globalMipLevel, ddsFormat);
        const uint32_t tiledMipLevelSize = xenos_texture::CalculateTiledLevelSize(
            image->width, image->height, localMipLevel, ddsFormat, streamBasePitch);

        if (rowPitch == 0 || ddsMipLevelSize == 0 || tiledMipLevelSize == 0)
        {
            PrintImageError("streamed image '%s' part %u has unsupported format %u at mip %u\n", image->name,
                            imagePartIndex, static_cast<uint32_t>(ddsFormat), localMipLevel);
            return false;
        }

        if (static_cast<size_t>(ddsOffset) + ddsMipLevelSize > ddsImage.data.size() ||
            destinationOffset + tiledMipLevelSize > static_cast<size_t>(image->cardMemory.platform[0]))
        {
            PrintImageError("streamed image '%s' part %u mip %u exceeds source or destination bounds\n", image->name,
                            imagePartIndex, localMipLevel);
            return false;
        }

        std::vector<uint8_t> levelData(ddsImage.data.begin() + ddsOffset,
                                       ddsImage.data.begin() + ddsOffset + ddsMipLevelSize);
        xenos_texture::ApplyGpuEndian(&levelData[0], levelData.size(),
                                      static_cast<GPUENDIAN>(image->texture.basemap.Format.Endian));

        std::vector<uint8_t> tiledData(tiledMipLevelSize);
        if (!xenos_texture::TileTextureLevel(image->width, image->height, localMipLevel, ddsFormat, streamBasePitch,
                                             &tiledData[0], tiledData.size(), &levelData[0], levelData.size(),
                                             rowPitch))
        {
            PrintImageError("failed to tile streamed image '%s' part %u mip %u\n", image->name, imagePartIndex,
                            localMipLevel);
            return false;
        }

        memcpy(image->pixels + destinationOffset, &tiledData[0], tiledMipLevelSize);
        ddsOffset += ddsMipLevelSize;
        destinationOffset += tiledMipLevelSize;
    }

    return true;
}

void TryReplaceStreamPart(game::GfxImage *image, uint32_t imagePartIndex)
{
    if (image == NULL || image->name == NULL)
        return;

    const std::string replacementPath = GetReplacementPath(image->name);
    const bool replacementExists = ImageFileExists(replacementPath);
    if (g_debugStreamImageCount < DEBUG_STREAM_IMAGE_LIMIT || replacementExists || imagePartIndex >= 4u)
    {
        DebugLog("ImageCache_InitImage image='%s' part=%u replacement=%u path='%s' mapType=%u size=%ux%u levels=%u "
                 "streaming=%u pixels=%p card=%d streamPixelSize=0x%08X\n",
                 image->name, imagePartIndex, replacementExists ? 1u : 0u, replacementPath.c_str(),
                 static_cast<uint32_t>(image->mapType), static_cast<uint32_t>(image->width),
                 static_cast<uint32_t>(image->height), static_cast<uint32_t>(image->levelCount),
                 image->streaming ? 1u : 0u, image->pixels, image->cardMemory.platform[0],
                 imagePartIndex < 4u ? image->streams[imagePartIndex].pixelSize : 0u);
    }
    g_debugStreamImageCount++;

    if (imagePartIndex >= 4u)
        return;

    if (!replacementExists)
        return;

    const DDSImage ddsImage = ReadDDSFile(replacementPath);
    if (Image_Replace_StreamPart(image, ddsImage, imagePartIndex))
        PrintImageInfo("replaced image '%s' (streamed part %u)\n", image->name, imagePartIndex);
}

void OnDBLinkXAssetPre(game::XAssetType &type, game::XAssetHeader *header)
{
    if (type == game::ASSET_TYPE_IMAGE && header != NULL)
    {
        game::GfxImage *image = header->image;
        if (image != NULL && image->name != NULL)
        {
            const std::string replacementPath = GetReplacementPath(image->name);
            const bool replacementExists = ImageFileExists(replacementPath);
            if (g_debugLinkedImageCount < DEBUG_LINKED_IMAGE_LIMIT || replacementExists)
                DebugLogImageState("DB_LinkXAssetPre", image, replacementExists, replacementPath);
            g_debugLinkedImageCount++;
        }

        Image_Replace(header->image);
    }
}

Detour ImageCache_InitImage_Detour;

void ImageCache_InitImage_Hook(game::GfxImage *image, game::GfxImage *remoteImage, unsigned __int8 *pixels,
                               unsigned int imagePartIndex)
{
    ImageCache_InitImage_Detour.GetOriginal<game::ImageCache_InitImage_t>()(image, remoteImage, pixels, imagePartIndex);
    TryReplaceStreamPart(image, imagePartIndex);
}
} // namespace

image_loader::image_loader()
{
    DebugLog("construct activeMod='%s' modBase='%s'\n", Config::active_mod.c_str(), Config::GetModBasePath().c_str());
    DebugListReplacementFiles();

    Events::OnDBLinkXAssetPre(OnDBLinkXAssetPre);
    DebugLog("registered DB_LinkXAssetPre image callback\n");

    ImageCache_InitImage_Detour = Detour(iw4::mp_tu6::ImageCache_InitImage, ImageCache_InitImage_Hook);
    ImageCache_InitImage_Detour.Install();
    DebugLog("installed ImageCache_InitImage detour target=%p\n", iw4::mp_tu6::ImageCache_InitImage);
}

image_loader::~image_loader()
{
    ImageCache_InitImage_Detour.Remove();
}
