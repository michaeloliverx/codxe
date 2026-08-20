#include "pch.h"
#include "common/endian.h"
#include "common/config.h"
#include "command.h"
#include "image_loader.h"
#include "image/xenos_texture.h"

// Forgive me for this dreadful code. It was hacked together until semi working and not touched since.
// TODO: refactor and generalise for the other games.

namespace
{
// DDS Constants
const uint32_t DDS_MAGIC = MAKEFOURCC('D', 'D', 'S', ' ');
const uint32_t DDS_HEADER_SIZE = 124;
const uint32_t DDS_PIXEL_FORMAT_SIZE = 32;
const uint32_t DDSD_CAPS = 0x1;
const uint32_t DDSD_HEIGHT = 0x2;
const uint32_t DDSD_WIDTH = 0x4;
const uint32_t DDSD_PIXELFORMAT = 0x1000;
const uint32_t DDSD_MIPMAPCOUNT = 0x20000;
const uint32_t DDSD_LINEARSIZE = 0x80000;
const uint32_t DDPF_FOURCC = 0x4;
const uint32_t DDPF_RGB = 0x40;
const uint32_t DDPF_ALPHAPIXELS = 0x1;
const uint32_t DDSCAPS_TEXTURE = 0x1000;
const uint32_t DDSCAPS_COMPLEX = 0x8;
const uint32_t DDSCAPS_MIPMAP = 0x400000;
const uint32_t DDPF_LUMINANCE = 0x20000;

// DDS Pixel Formats (FourCC Codes)
const uint32_t DXT1_FOURCC = MAKEFOURCC('D', 'X', 'T', '1');
const uint32_t DXT3_FOURCC = MAKEFOURCC('D', 'X', 'T', '3');
const uint32_t DXT5_FOURCC = MAKEFOURCC('D', 'X', 'T', '5');
const uint32_t DXN_FOURCC = MAKEFOURCC('A', 'T', 'I', '2'); // (DXN / BC5)

// Additional DDS Cubemap Flags
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

// DDS Header Structure
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

void SwapDDSHeaderEndian(DDSHeader &header)
{
    header.magic = endian::ByteSwap(header.magic);
    header.size = endian::ByteSwap(header.size);
    header.flags = endian::ByteSwap(header.flags);
    header.height = endian::ByteSwap(header.height);
    header.width = endian::ByteSwap(header.width);
    header.pitchOrLinearSize = endian::ByteSwap(header.pitchOrLinearSize);
    header.depth = endian::ByteSwap(header.depth);
    header.mipMapCount = endian::ByteSwap(header.mipMapCount);

    for (int i = 0; i < 11; i++)
        header.reserved1[i] = endian::ByteSwap(header.reserved1[i]);

    header.pixelFormat.size = endian::ByteSwap(header.pixelFormat.size);
    header.pixelFormat.flags = endian::ByteSwap(header.pixelFormat.flags);
    header.pixelFormat.fourCC = endian::ByteSwap(header.pixelFormat.fourCC);
    header.pixelFormat.rgbBitCount = endian::ByteSwap(header.pixelFormat.rgbBitCount);
    header.pixelFormat.rBitMask = endian::ByteSwap(header.pixelFormat.rBitMask);
    header.pixelFormat.gBitMask = endian::ByteSwap(header.pixelFormat.gBitMask);
    header.pixelFormat.bBitMask = endian::ByteSwap(header.pixelFormat.bBitMask);
    header.pixelFormat.aBitMask = endian::ByteSwap(header.pixelFormat.aBitMask);

    header.caps = endian::ByteSwap(header.caps);
    header.caps2 = endian::ByteSwap(header.caps2);
    header.caps3 = endian::ByteSwap(header.caps3);
    header.caps4 = endian::ByteSwap(header.caps4);
    header.reserved2 = endian::ByteSwap(header.reserved2);
}

DDSImage ReadDDSFile(const std::string &filepath)
{
    DDSImage ddsImage;
    std::ifstream file(filepath, std::ios::binary);

    if (!file.is_open())
    {
        return ddsImage; // Return empty DDSImage
    }

    // Read DDS header (raw, little-endian)
    file.read(reinterpret_cast<char *>(&ddsImage.header), sizeof(DDSHeader));
    if (!file || file.gcount() != sizeof(DDSHeader))
    {
        file.close();
        return ddsImage;
    }

    // Swap only the magic number to big-endian for proper validation
    uint32_t magicSwapped = endian::ByteSwap(ddsImage.header.magic);

    if (magicSwapped != 0x20534444) // 'DDS ' in big-endian
    {
        file.close();
        return ddsImage;
    }

    // Swap header fields to big-endian for Xbox 360
    SwapDDSHeaderEndian(ddsImage.header);

    // Move to end of file to get total file size
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();

    // Ensure fileSize is valid before proceeding
    if (fileSize == std::streampos(-1))
    {
        file.close();
        return ddsImage;
    }

    // Move back to after the header
    file.seekg(sizeof(DDSHeader), std::ios::beg);

    // Compute data size safely
    size_t dataSize = static_cast<size_t>(fileSize) - sizeof(DDSHeader);

    // Read image data
    ddsImage.data.resize(dataSize);
    if (dataSize > 0)
    {
        file.read(reinterpret_cast<char *>(ddsImage.data.data()), dataSize);
        if (!file || static_cast<size_t>(file.gcount()) != dataSize)
        {
            ddsImage.data.clear();
            file.close();
            return ddsImage;
        }
    }

    file.close();

    return ddsImage;
}

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

bool DDSIsCubemap(const DDSImage &ddsImage)
{
    return (ddsImage.header.caps2 & DDSCAPS2_CUBEMAP) != 0 ||
           (ddsImage.header.caps2 & DDSCAPS2_CUBEMAP_ALL_FACES) == DDSCAPS2_CUBEMAP_ALL_FACES;
}

uint32_t GetDDSMipCount(const DDSImage &ddsImage)
{
    return max(1u, static_cast<uint32_t>(ddsImage.header.mipMapCount));
}

bool GetDDSFormat(const DDSImage &ddsImage, GPUTEXTUREFORMAT *format)
{
    if ((ddsImage.header.pixelFormat.flags & DDPF_FOURCC) == 0)
        return false;

    switch (ddsImage.header.pixelFormat.fourCC)
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

    for (uint32_t currentMipLevel = 0; currentMipLevel < mipLevel; ++currentMipLevel)
    {
        const uint32_t levelSize =
            image::xenos_texture::CalculateLinearLevelSize(width, height, currentMipLevel, format);
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

bool Validate2DReplacementData(const iw3::mp::GfxImage *image, const DDSImage &ddsImage, GPUTEXTUREFORMAT format,
                               uint32_t ddsFirstMipLevel, uint32_t replacementLevelCount, size_t *requiredDDSSize,
                               size_t *requiredTextureBytes)
{
    const size_t ddsMipOffset =
        CalculateDDSMipOffset(ddsImage.header.width, ddsImage.header.height, format, ddsFirstMipLevel);
    const size_t requiredLinearSize = CalculateRequiredLinearDataSize(
        ddsImage.header.width, ddsImage.header.height, format, ddsFirstMipLevel, replacementLevelCount, 1u);
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

bool ValidateCubeReplacementData(const iw3::mp::GfxImage *image, const DDSImage &ddsImage, GPUTEXTUREFORMAT format,
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

bool BuildDDSHeader(DDSHeader *header, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipCount,
                    uint32_t linearSize, GPUTEXTUREFORMAT format, bool cubemap)
{
    memset(header, 0, sizeof(*header));

    uint32_t flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE;
    uint32_t caps = DDSCAPS_TEXTURE;
    if (mipCount > 1u)
    {
        flags |= DDSD_MIPMAPCOUNT;
        caps |= DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
    }

    header->magic = endian::ByteSwap(DDS_MAGIC);
    header->size = endian::ByteSwap(DDS_HEADER_SIZE);
    header->flags = endian::ByteSwap(flags);
    header->height = endian::ByteSwap(height);
    header->width = endian::ByteSwap(width);
    header->pitchOrLinearSize = endian::ByteSwap(linearSize);
    header->depth = endian::ByteSwap(depth);
    header->mipMapCount = endian::ByteSwap(mipCount);
    header->pixelFormat.size = endian::ByteSwap(DDS_PIXEL_FORMAT_SIZE);
    header->caps = endian::ByteSwap(caps);

    switch (format)
    {
    case GPUTEXTUREFORMAT_DXT1:
        header->pixelFormat.flags = endian::ByteSwap(DDPF_FOURCC);
        header->pixelFormat.fourCC = endian::ByteSwap(DXT1_FOURCC);
        break;
    case GPUTEXTUREFORMAT_DXT2_3:
        header->pixelFormat.flags = endian::ByteSwap(DDPF_FOURCC);
        header->pixelFormat.fourCC = endian::ByteSwap(DXT3_FOURCC);
        break;
    case GPUTEXTUREFORMAT_DXT4_5:
        header->pixelFormat.flags = endian::ByteSwap(DDPF_FOURCC);
        header->pixelFormat.fourCC = endian::ByteSwap(DXT5_FOURCC);
        break;
    case GPUTEXTUREFORMAT_DXN:
        header->pixelFormat.flags = endian::ByteSwap(DDPF_FOURCC);
        header->pixelFormat.fourCC = endian::ByteSwap(DXN_FOURCC);
        break;
    case GPUTEXTUREFORMAT_8:
        header->pixelFormat.flags = endian::ByteSwap(DDPF_LUMINANCE);
        header->pixelFormat.rgbBitCount = endian::ByteSwap(8u);
        header->pixelFormat.rBitMask = endian::ByteSwap(0x000000FFu);
        break;
    case GPUTEXTUREFORMAT_8_8:
        header->pixelFormat.flags = endian::ByteSwap(DDPF_LUMINANCE | DDPF_ALPHAPIXELS);
        header->pixelFormat.rgbBitCount = endian::ByteSwap(16u);
        header->pixelFormat.rBitMask = endian::ByteSwap(0x000000FFu);
        header->pixelFormat.gBitMask = endian::ByteSwap(0x0000FF00u);
        break;
    case GPUTEXTUREFORMAT_8_8_8_8:
        header->pixelFormat.flags = endian::ByteSwap(DDPF_RGB | DDPF_ALPHAPIXELS);
        header->pixelFormat.rgbBitCount = endian::ByteSwap(32u);
        header->pixelFormat.rBitMask = endian::ByteSwap(0x00FF0000u);
        header->pixelFormat.gBitMask = endian::ByteSwap(0x0000FF00u);
        header->pixelFormat.bBitMask = endian::ByteSwap(0x000000FFu);
        header->pixelFormat.aBitMask = endian::ByteSwap(0xFF000000u);
        break;
    default:
        return false;
    }

    if (cubemap)
    {
        header->caps = endian::ByteSwap(caps | DDSCAPS_COMPLEX);
        header->caps2 = endian::ByteSwap(DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_ALL_FACES);
    }

    return true;
}

std::string GetImageRelativePath(const char *imageName)
{
    std::string fileName = imageName;
    fileName += ".dds";
    return filesystem::JoinPath("images", fileName.c_str());
}

std::string GetImageDumpPath(const char *imageName)
{
    std::string sanitizedName = imageName;
    sanitizedName.erase(std::remove_if(sanitizedName.begin(), sanitizedName.end(), [](char c) { return c == '*'; }),
                        sanitizedName.end());
    const std::string relativePath = GetImageRelativePath(sanitizedName.c_str());
    return filesystem::JoinPath(DUMP_DIR, relativePath.c_str());
}

bool ReadBinaryFile(const std::string &path, std::vector<uint8_t> *data)
{
    std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
    if (!file)
        return false;

    const std::streamsize size = file.tellg();
    if (size < 0)
        return false;

    file.seekg(0, std::ios::beg);
    data->resize(static_cast<size_t>(size));
    return size == 0 || file.read(reinterpret_cast<char *>(data->data()), size) != 0;
}

bool UntileImageLevel(uint32_t width, uint32_t height, GPUTEXTUREFORMAT format, uint32_t basePitch, GPUENDIAN endian,
                      const uint8_t *source, size_t sourceSize, std::vector<uint8_t> *linearData)
{
    const uint32_t tiledSize = image::xenos_texture::CalculateTiledLevelSize(width, height, 0u, format, basePitch);
    const uint32_t linearSize = image::xenos_texture::CalculateLinearLevelSize(width, height, 0u, format);
    const uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(width, 0u, format);
    if (tiledSize == 0 || linearSize == 0 || rowPitch == 0 || source == NULL || sourceSize < tiledSize)
        return false;

    std::vector<uint8_t> tiledData(source, source + tiledSize);
    image::xenos_texture::ApplyGpuEndian(tiledData.data(), tiledData.size(), endian);

    linearData->resize(linearSize);
    return image::xenos_texture::UntileTextureLevel(width, height, 0u, format, basePitch, linearData->data(),
                                                    linearData->size(), rowPitch, tiledData.data(), tiledData.size());
}

} // namespace

namespace iw3
{
namespace mp
{
void Image_DbgPrint(const GfxImage *image)
{
    const int format = image->texture.basemap->Format.DataFormat;
    char *format_str;
    switch (format)
    {
    case GPUTEXTUREFORMAT_DXT1:
        format_str = "DXT1";
        break;
    case GPUTEXTUREFORMAT_DXT2_3:
        format_str = "DXT2_3";
        break;
    case GPUTEXTUREFORMAT_DXT4_5:
        format_str = "DXT4_5";
        break;
    case GPUTEXTUREFORMAT_DXN:
        format_str = "DXN";
        break;
    case GPUTEXTUREFORMAT_8:
        format_str = "8";
        break;
    case GPUTEXTUREFORMAT_8_8:
        format_str = "8_8";
        break;
    case GPUTEXTUREFORMAT_8_8_8_8:
        format_str = "8_8_8_8";
        break;
    default:
        format_str = "UNKNOWN";
        break;
    }

    const image::xenos_texture::TextureFormatInfo *formatInfo =
        image::xenos_texture::GetTextureFormatInfo(image->texture.basemap->Format.DataFormat);
    const uint32_t MipTailBaseLevel = image::xenos_texture::GetMipTailBaseLevel(image->width, image->height);
    const uint32_t MipLevelCount = image::xenos_texture::GetTextureLevelCount(image->texture.basemap);
    const uint32_t faceCount = image->mapType == MAPTYPE_CUBE ? 6u : 1u;
    const uint32_t BaseSize =
        image::xenos_texture::CalculateBaseSize(image->texture.basemap, image->width, image->height, faceCount);
    const uint32_t bitsPerPixel = formatInfo != NULL ? formatInfo->bitsPerPixel : 0u;
    const uint32_t bytesPerBlock = formatInfo != NULL ? formatInfo->bytesPerBlock : 0u;

    Com_Printf(CON_CHANNEL_CONSOLEONLY,
               "Image_DbgPrint: Dumping image Name='%s', Type=%d, Dimensions=%dx%d, MipLevels=%d, MipTailBaseLevel=%d, "
               "Format=%s, BitsPerPixel=%d, BytesPerBlock=%d, Endian=%d, BaseSize=%d\n",
               image->name, image->mapType, image->width, image->height, MipLevelCount, MipTailBaseLevel, format_str,
               bitsPerPixel, bytesPerBlock, image->texture.basemap->Format.Endian, BaseSize);
}

void Image_Dump(const GfxImage *image)
{
    // TODO: cleanup empty files if failed

    if (!image)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Null GfxImage!\n");
        return;
    }

    Com_Printf(CON_CHANNEL_CONSOLEONLY, "Image_Dump: Dumping image '%s'\n", image->name);

    if (!image->pixels || image->baseSize == 0)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Image '%s' has no valid pixel data!\n", image->name);
        return;
    }

    if (image->mapType != MAPTYPE_2D && image->mapType != MAPTYPE_CUBE)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Unsupported map type %d!\n", image->mapType);
        return;
    }

    const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(image->texture.basemap->Format.DataFormat);
    const uint32_t linearSize = image::xenos_texture::CalculateLinearLevelSize(image->width, image->height, 0u, format);
    DDSHeader header;
    if (linearSize == 0 || !BuildDDSHeader(&header, image->width, image->height, image->depth, 1u, linearSize, format,
                                           image->mapType == MAPTYPE_CUBE))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Unsupported texture format %d!\n", format);
        return;
    }

    const std::string filename = GetImageDumpPath(image->name);

    std::ofstream file(filename, std::ios::binary);
    if (!file)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Failed to open file: %s\n", filename.c_str());
        return;
    }

    if (image->mapType == MAPTYPE_CUBE)
    {
        file.write(reinterpret_cast<const char *>(&header), sizeof(DDSHeader));

        const uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(image->width, 0u, format);
        const uint32_t tiledFaceSize = image::xenos_texture::CalculateTiledLevelSize(
            image->width, image->height, 0u, format, image->texture.basemap->Format.Pitch);
        if (rowPitch == 0 || tiledFaceSize == 0 || image->baseSize < tiledFaceSize * 6u)
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Invalid cube map layout for image '%s'\n", image->name);
            return;
        }

        // TODO: handle mip levels per face for cubemaps
        for (int i = 0; i < 6; i++)
        {
            unsigned char *facePixels = image->pixels + (i * tiledFaceSize);

            std::vector<uint8_t> swappedFace(facePixels, facePixels + tiledFaceSize);
            image::xenos_texture::ApplyGpuEndian(swappedFace.data(), swappedFace.size(),
                                                 static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

            std::vector<uint8_t> linearFace(linearSize);

            if (!image::xenos_texture::UntileTextureLevel(
                    image->width, image->height, 0, static_cast<uint32_t>(format), image->texture.basemap->Format.Pitch,
                    linearFace.data(), linearFace.size(), rowPitch, swappedFace.data(), swappedFace.size()))
            {
                Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Failed to untile cube image '%s' face %d\n", image->name,
                               i);
                return;
            }

            file.write(reinterpret_cast<const char *>(linearFace.data()), linearFace.size());
        }

        file.close();
    }
    else if (image->mapType == MAPTYPE_2D)
    {
        file.write(reinterpret_cast<const char *>(&header), sizeof(DDSHeader));

        std::vector<uint8_t> linearData;
        if (!UntileImageLevel(image->width, image->height, format, image->texture.basemap->Format.Pitch,
                              static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian), image->pixels,
                              image->baseSize, &linearData))
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Failed to untile image '%s'\n", image->name);
            return;
        }

        file.write(reinterpret_cast<const char *>(linearData.data()), linearData.size());

        file.close();
    }
    else
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Unsupported map type %d!\n", image->mapType);
        return;
    }
}

bool Image_DumpStreamed(const GfxImage *image, const std::string &highMipPath)
{
    if (image == NULL || image->name == NULL || image->texture.basemap == NULL || image->pixels == NULL ||
        image->mapType != MAPTYPE_2D || !image->streaming)
    {
        return false;
    }

    const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(image->texture.basemap->Format.DataFormat);
    if (format != GPUTEXTUREFORMAT_DXT1 && format != GPUTEXTUREFORMAT_DXT2_3 && format != GPUTEXTUREFORMAT_DXT4_5 &&
        format != GPUTEXTUREFORMAT_DXN)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Streamed image '%s' has unsupported format %d\n", image->name,
                       format);
        return false;
    }

    std::vector<uint8_t> highMipData;
    if (!ReadBinaryFile(highMipPath, &highMipData))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Failed to read streamed image '%s': %s\n", image->name,
                       highMipPath.c_str());
        return false;
    }

    const uint32_t streamedWidth = static_cast<uint32_t>(image->width) * 2u;
    const uint32_t streamedHeight = static_cast<uint32_t>(image->height) * 2u;
    const GPUENDIAN endian = static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian);

    std::vector<uint8_t> streamedLinearData;
    if (!UntileImageLevel(streamedWidth, streamedHeight, format, 0u, endian, highMipData.data(), highMipData.size(),
                          &streamedLinearData))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Failed to untile streamed image '%s'\n", image->name);
        return false;
    }

    std::vector<uint8_t> residentLinearData;
    if (!UntileImageLevel(image->width, image->height, format, image->texture.basemap->Format.Pitch, endian,
                          image->pixels, image->baseSize, &residentLinearData))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Failed to untile resident image '%s'\n", image->name);
        return false;
    }

    DDSHeader header;
    if (!BuildDDSHeader(&header, streamedWidth, streamedHeight, image->depth, 2u,
                        static_cast<uint32_t>(streamedLinearData.size()), format, false))
    {
        return false;
    }

    const std::string outputPath = GetImageDumpPath(image->name);
    std::ofstream outputFile(outputPath.c_str(), std::ios::binary);
    if (!outputFile)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Failed to open file: %s\n", outputPath.c_str());
        return false;
    }

    outputFile.write(reinterpret_cast<const char *>(&header), sizeof(header));
    outputFile.write(reinterpret_cast<const char *>(streamedLinearData.data()), streamedLinearData.size());
    outputFile.write(reinterpret_cast<const char *>(residentLinearData.data()), residentLinearData.size());
    if (!outputFile)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Failed to write streamed image '%s'\n", image->name);
        return false;
    }

    Com_Printf(CON_CHANNEL_CONSOLEONLY, "Image_Dump: Dumped unified streamed image '%s'\n", image->name);
    return true;
}

void Cmd_imagedump()
{
    ImageList imageList;
    R_GetImageList(&imageList);

    const std::string imageDumpDirectory = filesystem::JoinPath(DUMP_DIR, "images");
    filesystem::CreateDirectories(imageDumpDirectory.c_str());

    for (unsigned int i = 0; i < imageList.count; ++i)
    {
        GfxImage *image = imageList.image[i];
        if (image == NULL)
            continue;

        Image_DbgPrint(image);
        Image_Dump(image);
    }

    const std::vector<std::string> highMipFiles = filesystem::ListFilesInDirectory("D:\\highmip");
    for (size_t i = 0; i < highMipFiles.size(); ++i)
    {
        const std::string highMipPath = "D:\\highmip\\" + highMipFiles[i];
        const std::string assetName = extract_filename(highMipFiles[i].c_str());
        XAssetEntryPoolEntry *asset = DB_FindXAssetEntry(ASSET_TYPE_IMAGE, assetName.c_str());
        if (asset == NULL)
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Streamed image '%s' was not found\n", assetName.c_str());
            continue;
        }

        Image_DumpStreamed(asset->entry.asset.header.image, highMipPath);
    }
}

bool Image_Replace_2D(GfxImage *image, const DDSImage &ddsImage, uint32_t ddsFirstMipLevel)
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
    const uint32_t ddsMipCount = GetDDSMipCount(ddsImage);
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

    uint32_t ddsOffset = CalculateDDSMipOffset(ddsImage.header.width, ddsImage.header.height, format, ddsFirstMipLevel);

    for (uint32_t localMipLevel = 0; localMipLevel < nonPackedLevelCount; localMipLevel++)
    {
        const uint32_t ddsMipLevel = ddsFirstMipLevel + localMipLevel;
        uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(ddsImage.header.width, ddsMipLevel, format);
        uint32_t ddsMipLevelSize = image::xenos_texture::CalculateLinearLevelSize(
            ddsImage.header.width, ddsImage.header.height, ddsMipLevel, format);
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

bool Image_Replace_Cube(GfxImage *image, const DDSImage &ddsImage)
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
    const std::string relativePath = GetImageRelativePath(image->name);
    const std::string replacement_path = Config::ResolveModPath(relativePath.c_str());

    if (!filesystem::FileExists(replacement_path.c_str()))
    {
        return;
    }

    DDSImage ddsImage = ReadDDSFile(replacement_path.c_str());
    if (ddsImage.data.empty())
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Failed to load DDS file: %s\n", replacement_path.c_str());
        return;
    }

    if (ddsImage.header.size != DDS_HEADER_SIZE || ddsImage.header.pixelFormat.size != DDS_PIXEL_FORMAT_SIZE)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' has an invalid DDS header: size=%u pixelFormatSize=%u\n",
                       image->name, ddsImage.header.size, ddsImage.header.pixelFormat.size);
        return;
    }

    GPUTEXTUREFORMAT ddsFormat;
    if (!GetDDSFormat(ddsImage, &ddsFormat))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' has an unsupported DDS format: flags=0x%X fourCC=0x%X\n",
                       image->name, ddsImage.header.pixelFormat.flags, ddsImage.header.pixelFormat.fourCC);
        return;
    }

    if (static_cast<uint32_t>(image->texture.basemap->Format.DataFormat) != static_cast<uint32_t>(ddsFormat))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' format does not match DDS file: Expected %d, Got %d\n",
                       image->name, static_cast<uint32_t>(image->texture.basemap->Format.DataFormat),
                       static_cast<uint32_t>(ddsFormat));
        return;
    }

    const bool ddsIsCubemap = DDSIsCubemap(ddsImage);
    const bool ddsMatchesImageDimensions =
        image->width == ddsImage.header.width && image->height == ddsImage.header.height;
    const bool ddsIncludesStreamedMip = image->streaming && image->mapType == MAPTYPE_2D && !ddsIsCubemap &&
                                        ddsImage.header.width == static_cast<uint32_t>(image->width) * 2u &&
                                        ddsImage.header.height == static_cast<uint32_t>(image->height) * 2u;
    uint32_t ddsFirstMipLevel = 0;

    if (image->mapType == MAPTYPE_2D && ddsIsCubemap)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' is 2D but replacement DDS is a cubemap!\n", image->name);
        return;
    }

    if (ddsIncludesStreamedMip)
    {
        const uint32_t ddsMipCount = GetDDSMipCount(ddsImage);
        if (ddsMipCount < 2u)
        {
            Com_PrintError(CON_CHANNEL_ERROR,
                           "Image '%s' replacement DDS contains a streamed level but no resident level: "
                           "%ux%u mipCount=%u\n",
                           image->name, ddsImage.header.width, ddsImage.header.height, ddsMipCount);
            return;
        }

        ddsFirstMipLevel = 1u;
    }
    else if (image->streaming && image->mapType == MAPTYPE_2D && ddsMatchesImageDimensions)
    {
        Com_PrintError(CON_CHANNEL_ERROR,
                       "Streamed image '%s' replacement has no streamed mip; stock high-mip data will overwrite it "
                       "when streamed. Use a %ux%u DDS with at least 2 mip levels.\n",
                       image->name, static_cast<uint32_t>(image->width) * 2u,
                       static_cast<uint32_t>(image->height) * 2u);
    }
    else if (!ddsMatchesImageDimensions)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Image '%s' dimensions do not match DDS file: image=%ux%u dds=%ux%u %s\n",
                       image->name, image->width, image->height, ddsImage.header.width, ddsImage.header.height,
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
        Com_Printf(CON_CHANNEL_CONSOLEONLY, "Replaced image '%s'\n", image->name);
    }
}

void image_loader::OnAssetLink(XAsset *asset)
{
    if (asset->type != ASSET_TYPE_IMAGE || !asset->header.image || !asset->header.image->name)
    {
        return;
    }

    GfxImage *image = asset->header.image;
    if (!image->delayLoadPixels)
    {
        Image_Replace(image);
    }
}

namespace
{
void ReplaceDelayedImage(void *const asset, void *)
{
    auto *const image = static_cast<GfxImage *>(asset);
    if (image != nullptr && image->delayLoadPixels)
    {
        Image_Replace(image);
    }
}
} // namespace

void image_loader::OnDelayStreamLoad()
{
    DB_EnumXAssets_FastFile(ASSET_TYPE_IMAGE, ReplaceDelayedImage, nullptr, false);
}

bool R_StreamLoadImageReplacement(const char *filename, unsigned int bytesToRead, unsigned __int8 *outData)
{
    const std::string assetName = extract_filename(filename);
    XAssetEntryPoolEntry *asset = DB_FindXAssetEntry(ASSET_TYPE_IMAGE, assetName.c_str());

    if (!asset)
        return false;

    GfxImage *image = asset->entry.asset.header.image;
    if (image == NULL || image->name == NULL || image->texture.basemap == NULL || image->mapType != MAPTYPE_2D ||
        outData == NULL)
        return false;

    const std::string relativePath = GetImageRelativePath(image->name);
    const std::string replacementPath = Config::ResolveModPath(relativePath.c_str());
    if (!filesystem::FileExists(replacementPath.c_str()))
        return false;

    DDSImage ddsImage = ReadDDSFile(replacementPath);
    if (ddsImage.data.empty())
        return false;

    const uint32_t streamedWidth = static_cast<uint32_t>(image->width) * 2u;
    const uint32_t streamedHeight = static_cast<uint32_t>(image->height) * 2u;
    if (ddsImage.header.width != streamedWidth || ddsImage.header.height != streamedHeight)
        return false;

    if (ddsImage.header.size != DDS_HEADER_SIZE || ddsImage.header.pixelFormat.size != DDS_PIXEL_FORMAT_SIZE ||
        DDSIsCubemap(ddsImage) || GetDDSMipCount(ddsImage) < 2u)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Streamed image '%s' has an invalid replacement DDS: %s\n", image->name,
                       replacementPath.c_str());
        return false;
    }

    GPUTEXTUREFORMAT ddsFormat;
    if (!GetDDSFormat(ddsImage, &ddsFormat) ||
        static_cast<uint32_t>(ddsFormat) != image->texture.basemap->Format.DataFormat)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Streamed image '%s' replacement format does not match the game texture\n",
                       image->name);
        return false;
    }

    const uint32_t sourceSize =
        image::xenos_texture::CalculateLinearLevelSize(streamedWidth, streamedHeight, 0u, ddsFormat);
    const uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(streamedWidth, 0u, ddsFormat);
    const uint32_t tiledSize =
        image::xenos_texture::CalculateTiledLevelSize(streamedWidth, streamedHeight, 0u, ddsFormat, 0u);
    if (sourceSize == 0 || rowPitch == 0 || tiledSize == 0 || ddsImage.data.size() < sourceSize ||
        tiledSize != bytesToRead)
    {
        Com_PrintError(CON_CHANNEL_ERROR,
                       "Streamed image '%s' replacement size mismatch: source=%u tiled=%u requested=%u\n", image->name,
                       sourceSize, tiledSize, bytesToRead);
        return false;
    }

    std::vector<uint8_t> levelData(ddsImage.data.begin(), ddsImage.data.begin() + sourceSize);
    image::xenos_texture::ApplyGpuEndian(levelData.data(), levelData.size(),
                                         static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

    if (!image::xenos_texture::TileTextureLevel(streamedWidth, streamedHeight, 0u, ddsFormat, 0u, outData, bytesToRead,
                                                levelData.data(), levelData.size(), rowPitch))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Failed to tile streamed image '%s'\n", image->name);
        return false;
    }

    Com_Printf(CON_CHANNEL_CONSOLEONLY, "Replaced streamed image '%s'\n", image->name);
    return true;
}

Detour R_StreamLoadFileSynchronously_Detour;

int R_StreamLoadFileSynchronously_Hook(const char *filename, unsigned int bytesToRead, unsigned __int8 *outData)
{
    if (R_StreamLoadImageReplacement(filename, bytesToRead, outData))
    {
        return 1;
    }

    // Fallback to original path if modified path failed
    return R_StreamLoadFileSynchronously_Detour.GetOriginal<decltype(R_StreamLoadFileSynchronously)>()(
        filename, bytesToRead, outData);
}

image_loader::image_loader()
{
    // Load streamed texture replacements from the active mod folder.
    R_StreamLoadFileSynchronously_Detour = Detour(R_StreamLoadFileSynchronously, R_StreamLoadFileSynchronously_Hook);
    R_StreamLoadFileSynchronously_Detour.Install();

    command::add("imagedump", Cmd_imagedump);
}

image_loader::~image_loader()
{
    R_StreamLoadFileSynchronously_Detour.Remove();
}
} // namespace mp
} // namespace iw3
