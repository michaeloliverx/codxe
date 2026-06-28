#include "pch.h"
#include "common/config.h"
#include "events.h"
#include "image_loader.h"
#include "image/dds_loader.h"
#include "image/dds_writer.h"
#include "image/texture_layout.h"
#include "image/xenos_texture.h"

#include <fstream>

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD) - 1)
#endif

#ifdef PtrToUint
#undef PtrToUint
#endif

namespace
{
namespace game = iw4::mp_tu6;

const uint32_t STREAM_PIXEL_SIZE_MASK = 0x3FFFFFF;
const uint32_t MAX_STREAM_COMPRESSED_SIZE = 64u * 1024u * 1024u;
const uint32_t DDS_FILE_HEADER_SIZE = sizeof(uint32_t) + sizeof(image::DDS_HEADER);
const uint32_t REPLACEMENT_ROW_SCRATCH_SIZE = 64u * 1024u;

typedef image::DdsImage DDSImage;

struct ZlibStream
{
    unsigned __int8 *next_in;
    unsigned int avail_in;
    unsigned int total_in;
    unsigned __int8 *next_out;
    unsigned int avail_out;
    unsigned int total_out;
    char *msg;
    void *state;
    unsigned __int8 *(__fastcall *zalloc)(unsigned __int8 *opaque, unsigned int items, unsigned int size);
    void(__fastcall *zfree)(unsigned __int8 *opaque, unsigned __int8 *ptr);
    unsigned __int8 *opaque;
    int data_type;
};
static_assert(sizeof(ZlibStream) == 48, "");

game::dvar_t *dump_assets = nullptr;
unsigned char g_replacementRowScratch[REPLACEMENT_ROW_SCRATCH_SIZE];

struct DdsDataFile
{
    HANDLE handle;
    std::string path;
    uint32_t dataSize;

    DdsDataFile(const std::string &filePath, uint32_t fileDataSize)
        : handle(CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)),
          path(filePath), dataSize(fileDataSize)
    {
    }

    ~DdsDataFile()
    {
        if (handle != INVALID_HANDLE_VALUE)
            CloseHandle(handle);
    }

    bool IsValid() const
    {
        return handle != INVALID_HANDLE_VALUE;
    }

    bool SeekData(uint32_t dataOffset, uint32_t size) const
    {
        if (dataOffset > dataSize || size > dataSize - dataOffset)
        {
            game::Com_Printf(0, "DDS range outside file '%s': dataOffset=%u size=%u dataSize=%u\n", path.c_str(),
                             dataOffset, size, dataSize);
            return false;
        }

        const DWORD fileOffset = static_cast<DWORD>(DDS_FILE_HEADER_SIZE) + dataOffset;
        SetLastError(NO_ERROR);
        const DWORD seekResult = SetFilePointer(handle, fileOffset, nullptr, FILE_BEGIN);
        if (seekResult == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
        {
            game::Com_Printf(0, "DDS seek failed '%s': offset=%u error=0x%08X\n", path.c_str(), fileOffset,
                             GetLastError());
            return false;
        }

        return true;
    }
};

struct DdsRowReadState
{
    DdsDataFile *file;
    GPUENDIAN endian;
};

uint32_t PtrToUint(const void *ptr)
{
    return static_cast<uint32_t>(reinterpret_cast<UINT_PTR>(ptr));
}

void PrintImageError(const char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    _vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    buffer[sizeof(buffer) - 1] = '\0';

    game::Com_Printf(0, "%s", buffer);
}

void PrintImageInfo(const char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    _vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    buffer[sizeof(buffer) - 1] = '\0';

    game::Com_Printf(0, "%s", buffer);
}

bool ReadDdsTileRow(uint32_t rowIndex, unsigned char *rowBuffer, uint32_t rowPitch, void *userData)
{
    DdsRowReadState *state = static_cast<DdsRowReadState *>(userData);
    if (state == nullptr || state->file == nullptr || rowBuffer == nullptr || rowPitch == 0)
        return false;

    DWORD bytesRead = 0;
    if (!ReadFile(state->file->handle, rowBuffer, rowPitch, &bytesRead, nullptr) || bytesRead != rowPitch)
    {
        PrintImageError("DDS row read failed '%s': row=%u size=%u actual=%u error=0x%08X\n",
                        state->file->path.c_str(), rowIndex, rowPitch, bytesRead, GetLastError());
        return false;
    }

    image::xenos_texture::ApplyGpuEndian(rowBuffer, rowPitch, state->endian);
    return true;
}

bool TileDdsLevelToTexture(DdsDataFile &ddsFile, uint32_t dataOffset, uint32_t expectedDataSize,
                           uint32_t sourceWidth, uint32_t sourceHeight, uint32_t sourceMipLevel,
                           uint32_t destinationWidth, uint32_t destinationHeight, uint32_t destinationMipLevel,
                           GPUTEXTUREFORMAT format, uint32_t basePitch, unsigned char *destination,
                           uint32_t destinationSize, GPUENDIAN endian)
{
    if (destination == nullptr || destinationSize == 0)
        return false;

    const uint32_t sourceRowPitch =
        image::xenos_texture::CalculateLinearRowPitch(sourceWidth, sourceMipLevel, format);
    const uint32_t sourceLevelSize =
        image::xenos_texture::CalculateLinearLevelSize(sourceWidth, sourceHeight, sourceMipLevel, format);
    if (sourceRowPitch == 0 || sourceLevelSize == 0 || sourceLevelSize != expectedDataSize)
        return false;

    if (sourceRowPitch > sizeof(g_replacementRowScratch))
    {
        PrintImageError("DDS row scratch too small '%s': rowPitch=%u scratch=%u\n", ddsFile.path.c_str(),
                        sourceRowPitch, static_cast<unsigned int>(sizeof(g_replacementRowScratch)));
        return false;
    }

    if (!ddsFile.SeekData(dataOffset, expectedDataSize))
        return false;

    DdsRowReadState rowState = {&ddsFile, endian};
    memset(destination, 0, destinationSize);
    return image::xenos_texture::TileTextureLevelFromRows(
        destinationWidth, destinationHeight, destinationMipLevel, format, basePitch, destination, destinationSize,
        sourceRowPitch, g_replacementRowScratch, sizeof(g_replacementRowScratch), ReadDdsTileRow, &rowState);
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

std::string GetUserReplacementDirectory()
{
    return std::string(USERRAW_DIR) + "\\images";
}

bool ReadDDSHeader(const std::string &filepath, DDSImage *out, uint32_t *dataSize)
{
    if (out == nullptr)
        return false;

    *out = DDSImage();
    return image::LoadDdsHeaderFromFile(filepath, &out->header, dataSize);
}

std::string GetReplacementPath(const char *imageName)
{
    const std::string userPath = GetUserReplacementDirectory() + "\\" + imageName + ".dds";
    if (ImageFileExists(userPath))
        return userPath;

    return GetReplacementDirectory() + "\\" + imageName + ".dds";
}

bool ValidateDDSHeaderFields(const game::GfxImage *image, const DDSImage &ddsImage, GPUTEXTUREFORMAT *ddsFormat)
{
    if (ddsImage.header.dwSize != image::DDS_HEADER_SIZE ||
        ddsImage.header.ddspf.dwSize != image::DDS_PIXEL_FORMAT_SIZE)
    {
        PrintImageError("image '%s' has an invalid DDS header: size=%u pixelFormatSize=%u\n", image->name,
                        ddsImage.header.dwSize, ddsImage.header.ddspf.dwSize);
        return false;
    }

    if (!ddsImage.GetGpuFormat(ddsFormat))
    {
        PrintImageError("image '%s' has an unsupported DDS format: flags=0x%X fourCC=0x%X bitCount=%u\n", image->name,
                        ddsImage.header.ddspf.dwFlags, ddsImage.header.ddspf.dwFourCC,
                        ddsImage.header.ddspf.dwRGBBitCount);
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

bool Validate2DReplacementData(const game::GfxImage *image, GPUTEXTUREFORMAT format, size_t ddsDataSize,
                               uint32_t replacementLevelCount, size_t *requiredDDSSize, size_t *requiredTextureBytes)
{
    *requiredDDSSize =
        image::CalculateRequiredLinearDataSize(image->width, image->height, format, 0u, replacementLevelCount, 1u);
    if (*requiredDDSSize == 0)
        return false;

    if (ddsDataSize < *requiredDDSSize)
        return false;

    const D3DBaseTexture *texture = &image->texture.basemap;
    const uint32_t baseSize = image::xenos_texture::CalculateBaseSize(texture, image->width, image->height, 1u);
    const size_t mipBytes =
        image::CalculateRequiredMipTextureBytes(image->width, image->height, format, 1u, replacementLevelCount, 1u);

    *requiredTextureBytes = static_cast<size_t>(baseSize) + mipBytes;
    const int cardMemory = image->cardMemory.platform[0];
    if (cardMemory > 0 && *requiredTextureBytes > static_cast<size_t>(cardMemory))
        return false;

    return true;
}

bool ValidateResidentMipCount(const game::GfxImage *image, const DDSImage &ddsImage, uint32_t textureLevelCount)
{
    const uint32_t ddsMipCount = ddsImage.GetMipCount();
    if (ddsMipCount != textureLevelCount)
    {
        PrintImageError("image '%s' mip count does not match DDS: image=%u dds=%u\n", image->name, textureLevelCount,
                        ddsMipCount);
        return false;
    }

    return true;
}

bool ValidateDDSDataSize(const game::GfxImage *image, const image::DDS_HEADER &header, size_t dataSize,
                         GPUTEXTUREFORMAT format, uint32_t mipCount, uint32_t faceCount)
{
    const size_t expectedSize = image::CalculateRequiredLinearDataSize(
        header.dwWidth, header.dwHeight, format, 0u, mipCount, faceCount);
    if (expectedSize == 0)
    {
        PrintImageError("image '%s' has unsupported DDS dimensions or format\n", image->name);
        return false;
    }

    if (dataSize != expectedSize)
    {
        PrintImageError("image '%s' DDS data size does not match shape: have=%u expected=%u mips=%u faces=%u\n",
                        image->name, static_cast<unsigned int>(dataSize), static_cast<unsigned int>(expectedSize),
                        mipCount, faceCount);
        return false;
    }

    return true;
}

bool ValidateCubeReplacementData(const game::GfxImage *image, size_t ddsDataSize, GPUTEXTUREFORMAT format,
                                 uint32_t faceSize, uint32_t tiledBaseSize, size_t *requiredDDSSize)
{
    *requiredDDSSize = static_cast<size_t>(faceSize) * 6u;
    if (faceSize == 0 || *requiredDDSSize == 0)
        return false;

    if (ddsDataSize != *requiredDDSSize)
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
        if ((image->streams[imagePartIndex].pixelSize & STREAM_PIXEL_SIZE_MASK) != 0)
            return true;
    }

    return false;
}

GPUTEXTUREFORMAT GetImageGpuFormat(const game::GfxImage *image)
{
    return static_cast<GPUTEXTUREFORMAT>(image->texture.basemap.Format.DataFormat);
}

uint32_t GetImageBasePitch(const game::GfxImage *image, bool streamed)
{
    if (streamed)
        return 0u;

    return image->texture.basemap.Format.Pitch;
}

GPUENDIAN GetImageEndian(const game::GfxImage *image)
{
    return static_cast<GPUENDIAN>(image->texture.basemap.Format.Endian);
}

uint32_t GetImageLevelCount(const game::GfxImage *image, bool streamed)
{
    uint32_t levelCount = max(1u, static_cast<uint32_t>(image->levelCount));

    if (!streamed && image->texture.basemap.Format.PackedMips != 0)
    {
        const uint32_t mipTailBaseLevel = image::xenos_texture::GetMipTailBaseLevel(image->width, image->height);
        levelCount = max(1u, min(levelCount, mipTailBaseLevel));
    }

    return levelCount;
}

std::string GetSanitizedImageName(const char *imageName)
{
    if (imageName == nullptr)
        return std::string();

    std::string sanitizedName;
    for (const char *current = imageName; *current != '\0'; ++current)
    {
        const char c = *current;
        if (c == '*')
            continue;

        if (c == '/' || c == '\\' || c == ':' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
            sanitizedName.push_back('_');
        else
            sanitizedName.push_back(c);
    }

    return sanitizedName;
}

std::string GetDumpZoneName(const char *zoneName)
{
    const std::string sanitizedZoneName = GetSanitizedImageName(zoneName);
    return sanitizedZoneName.empty() ? "unknown" : sanitizedZoneName;
}

std::string GetImageDumpPath(const char *imageName, const char *zoneName)
{
    return std::string(DUMP_DIR) + "\\" + GetDumpZoneName(zoneName) + "\\images\\" + GetSanitizedImageName(imageName) +
           ".dds";
}

void EnsureImageDumpDirectory(const char *zoneName)
{
    CreateDirectoryA(DUMP_DIR, nullptr);

    const std::string zoneDirectory = std::string(DUMP_DIR) + "\\" + GetDumpZoneName(zoneName);
    CreateDirectoryA(zoneDirectory.c_str(), nullptr);
    CreateDirectoryA((zoneDirectory + "\\images").c_str(), nullptr);
}

const char *GetZoneName(uint32_t zoneIndex)
{
    if (zoneIndex >= game::g_zoneCount)
        return nullptr;

    const char *zoneName = game::g_zones[zoneIndex].file.name;
    if (zoneName[0] == '\0')
        return nullptr;

    return zoneName;
}

bool ReadFileRange(const std::string &path, uint32_t offset, uint32_t size, std::vector<uint8_t> *buffer)
{
    if (buffer == nullptr || size == 0)
        return false;

    HANDLE file = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;

    SetLastError(NO_ERROR);
    const DWORD seekResult = SetFilePointer(file, offset, nullptr, FILE_BEGIN);
    if (seekResult == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
    {
        CloseHandle(file);
        return false;
    }

    buffer->assign(size, 0);

    DWORD bytesRead = 0;
    const BOOL readOk = ReadFile(file, &(*buffer)[0], size, &bytesRead, nullptr);
    CloseHandle(file);

    if (!readOk || bytesRead != size)
    {
        buffer->clear();
        return false;
    }

    return true;
}

bool GetImagePoolIndex(const game::GfxImage *image, uint32_t *imageIndex)
{
    if (image == nullptr || imageIndex == nullptr)
        return false;

    const uint32_t imageAddress = PtrToUint(image);
    const uint32_t poolAddress = PtrToUint(game::g_gfxImagePool);
    const uint32_t poolSize = sizeof(game::GfxImage) * game::g_gfxImagePoolSize;
    if (imageAddress < poolAddress || imageAddress >= poolAddress + poolSize)
        return false;

    const uint32_t imageOffset = imageAddress - poolAddress;
    if ((imageOffset % sizeof(game::GfxImage)) != 0)
        return false;

    *imageIndex = imageOffset / sizeof(game::GfxImage);
    return true;
}

const game::GfxSubImageStream *GetImageStreamSources(const game::GfxImage *image)
{
    uint32_t imageIndex = 0;
    if (!GetImagePoolIndex(image, &imageIndex))
        return nullptr;

    return game::g_imageStreams[imageIndex].part;
}

bool GetImageFilePath(const game::GfxSubImageStream &source, std::string *path)
{
    if (path == nullptr || source.file == nullptr || source.file->name[0] == '\0')
        return false;

    char filePath[MAX_PATH];
    _snprintf(filePath, sizeof(filePath), "game:\\%s.pak", source.file->name);
    filePath[sizeof(filePath) - 1] = '\0';
    *path = filePath;
    return true;
}

unsigned __int8 *__fastcall ImageZlibAlloc(unsigned __int8 *opaque, unsigned int items, unsigned int size)
{
    (void)opaque;

    if (items == 0 || size == 0 || items > 0xFFFFFFFFu / size)
        return nullptr;

    return static_cast<unsigned __int8 *>(malloc(items * size));
}

void __fastcall ImageZlibFree(unsigned __int8 *opaque, unsigned __int8 *ptr)
{
    (void)opaque;
    free(ptr);
}

bool InflateImageStream(const std::vector<uint8_t> &compressedData, uint32_t expectedSize,
                        std::vector<uint8_t> *inflatedData)
{
    if (inflatedData == nullptr || compressedData.empty() || expectedSize == 0)
        return false;

    inflatedData->assign(expectedSize, 0);
    ZlibStream stream;
    memset(&stream, 0, sizeof(stream));

    stream.next_in = const_cast<unsigned __int8 *>(&compressedData[0]);
    stream.avail_in = static_cast<unsigned int>(compressedData.size());
    stream.next_out = &(*inflatedData)[0];
    stream.avail_out = expectedSize;
    stream.zalloc = ImageZlibAlloc;
    stream.zfree = ImageZlibFree;

    int result = game::Zlib_InflateInit(&stream, "1.1.4", sizeof(stream));
    if (result == 0)
    {
        const int inflateResult = game::Zlib_Inflate(&stream, 4);
        if (inflateResult == 1)
        {
            result = game::Zlib_InflateEnd(&stream);
        }
        else
        {
            game::Zlib_InflateEnd(&stream);
            result = inflateResult != 0 ? inflateResult : -5;
        }
    }

    if (result != 0 || stream.total_out == 0 || stream.total_out > expectedSize)
    {
        inflatedData->clear();
        return false;
    }

    inflatedData->resize(stream.total_out);
    return true;
}

bool WriteUntiledLevel(std::ofstream &file, const game::GfxImage *image, uint32_t width, uint32_t height,
                       uint32_t mipLevel, GPUTEXTUREFORMAT format, uint32_t basePitch, const unsigned char *tiledPixels,
                       uint32_t tiledSize, GPUENDIAN endian)
{
    const uint32_t linearLevelSize = image::xenos_texture::CalculateLinearLevelSize(width, height, mipLevel, format);
    const uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(width, mipLevel, format);
    if (linearLevelSize == 0 || rowPitch == 0 || tiledSize == 0)
        return false;

    std::vector<uint8_t> tiledData(tiledPixels, tiledPixels + tiledSize);
    image::xenos_texture::ApplyGpuEndian(&tiledData[0], tiledData.size(), endian);

    std::vector<uint8_t> linearData(linearLevelSize);
    if (!image::xenos_texture::UntileTextureLevel(width, height, mipLevel, format, basePitch, &linearData[0],
                                                  linearData.size(), rowPitch, &tiledData[0], tiledData.size()))
        return false;

    file.write(reinterpret_cast<const char *>(&linearData[0]), linearData.size());
    return true;
}

bool Dump2DImage(const game::GfxImage *image, bool streamed, const char *zoneName)
{
    if (image->pixels == nullptr || image->cardMemory.platform[0] <= 0)
        return false;

    const GPUTEXTUREFORMAT format = GetImageGpuFormat(image);
    const uint32_t levelCount = GetImageLevelCount(image, streamed);
    const uint32_t basePitch = GetImageBasePitch(image, streamed);
    const GPUENDIAN endian = GetImageEndian(image);
    const uint32_t linearBaseSize =
        image::xenos_texture::CalculateLinearLevelSize(image->width, image->height, 0u, format);
    const uint32_t tiledBaseSize =
        image::xenos_texture::CalculateTiledLevelSize(image->width, image->height, 0u, format, basePitch);
    if (linearBaseSize == 0 || tiledBaseSize == 0)
        return false;

    const size_t requiredBytes =
        static_cast<size_t>(tiledBaseSize) +
        image::CalculateRequiredMipTextureBytes(image->width, image->height, format, 1u, levelCount, 1u);
    if (requiredBytes > static_cast<size_t>(image->cardMemory.platform[0]))
        return false;

    image::DDS_HEADER header;
    const uint32_t caps =
        image::DDSCAPS_TEXTURE | (levelCount > 1u ? image::DDSCAPS_COMPLEX | image::DDSCAPS_MIPMAP : 0u);
    if (!image::CreateDdsHeader(header, image->width, image->height, image->depth, levelCount, linearBaseSize, caps, 0u,
                                format))
        return false;

    if (levelCount > 1u)
        header.dwFlags |= image::DDSD_MIPMAPCOUNT;

    EnsureImageDumpDirectory(zoneName);
    const std::string filename = GetImageDumpPath(image->name, zoneName);
    std::ofstream file(filename.c_str(), std::ios::binary);
    if (!file)
    {
        PrintImageError("Could not create DDS for image '%s': %s\n", image->name, filename.c_str());
        return false;
    }

    image::WriteDdsHeader(file, header);

    const unsigned char *baseData = image->pixels;
    const unsigned char *mipData = baseData + tiledBaseSize;
    for (uint32_t mipLevel = 0; mipLevel < levelCount; ++mipLevel)
    {
        const uint32_t tiledLevelSize =
            image::xenos_texture::CalculateTiledLevelSize(image->width, image->height, mipLevel, format, basePitch);
        const unsigned char *source = baseData;
        if (mipLevel > 0)
        {
            source = mipData +
                     image::xenos_texture::CalculateMipLevelOffset(image->width, image->height, mipLevel, format, 1u);
        }

        if (!WriteUntiledLevel(file, image, image->width, image->height, mipLevel, format, basePitch, source,
                               tiledLevelSize, endian))
            return false;
    }

    PrintImageInfo("Dumped image '%s'%s\n", image->name, streamed ? " (streamed)" : "");
    return true;
}

bool DumpCubeImage(const game::GfxImage *image, bool streamed, const char *zoneName)
{
    if (image->pixels == nullptr || image->cardMemory.platform[0] <= 0)
        return false;

    const GPUTEXTUREFORMAT format = GetImageGpuFormat(image);
    const uint32_t basePitch = GetImageBasePitch(image, streamed);
    const GPUENDIAN endian = GetImageEndian(image);
    const uint32_t linearFaceSize =
        image::xenos_texture::CalculateLinearLevelSize(image->width, image->height, 0u, format);
    const uint32_t tiledFaceSize =
        image::xenos_texture::CalculateTiledLevelSize(image->width, image->height, 0u, format, basePitch);
    if (linearFaceSize == 0 || tiledFaceSize == 0)
        return false;

    const size_t requiredBytes = static_cast<size_t>(tiledFaceSize) * 6u;
    if (requiredBytes > static_cast<size_t>(image->cardMemory.platform[0]))
        return false;

    const uint32_t caps2 = image::DDSCAPS2_CUBEMAP | image::DDSCAPS2_CUBEMAP_POSITIVEX |
                           image::DDSCAPS2_CUBEMAP_NEGATIVEX | image::DDSCAPS2_CUBEMAP_POSITIVEY |
                           image::DDSCAPS2_CUBEMAP_NEGATIVEY | image::DDSCAPS2_CUBEMAP_POSITIVEZ |
                           image::DDSCAPS2_CUBEMAP_NEGATIVEZ;

    image::DDS_HEADER header;
    if (!image::CreateDdsHeader(header, image->width, image->height, image->depth, 1u, linearFaceSize,
                                image::DDSCAPS_TEXTURE | image::DDSCAPS_COMPLEX, caps2, format))
        return false;

    EnsureImageDumpDirectory(zoneName);
    const std::string filename = GetImageDumpPath(image->name, zoneName);
    std::ofstream file(filename.c_str(), std::ios::binary);
    if (!file)
    {
        PrintImageError("Could not create DDS for image '%s': %s\n", image->name, filename.c_str());
        return false;
    }

    image::WriteDdsHeader(file, header);

    for (uint32_t faceIndex = 0; faceIndex < 6u; ++faceIndex)
    {
        const unsigned char *source = image->pixels + static_cast<size_t>(faceIndex) * tiledFaceSize;
        if (!WriteUntiledLevel(file, image, image->width, image->height, 0u, format, basePitch, source, tiledFaceSize,
                               endian))
            return false;
    }

    PrintImageInfo("Dumped image '%s'%s\n", image->name, streamed ? " (streamed)" : "");
    return true;
}

uint32_t GetStreamPartLevelCount(const game::GfxImage *image, uint32_t imagePartIndex)
{
    const uint32_t streamLevelCount = image->streams[imagePartIndex].pixelSize >> 26;
    if (streamLevelCount != 0)
        return streamLevelCount;

    return max(1u, static_cast<uint32_t>(image->levelCount));
}

bool DumpStreamPartFromData(const game::GfxImage *image, uint32_t imagePartIndex, const char *zoneName,
                            const std::vector<uint8_t> &pixelData)
{
    if (imagePartIndex >= 4u || pixelData.empty())
        return false;

    const game::GfxImageStreamData &streamData = image->streams[imagePartIndex];
    if (streamData.width == 0 || streamData.height == 0)
        return false;

    game::GfxImage streamImage = *image;
    streamImage.width = streamData.width;
    streamImage.height = streamData.height;
    streamImage.levelCount = static_cast<unsigned char>(GetStreamPartLevelCount(image, imagePartIndex));
    streamImage.cardMemory.platform[0] = static_cast<int>(pixelData.size());
    streamImage.pixels = const_cast<unsigned char *>(&pixelData[0]);

    if (streamImage.mapType == game::MAPTYPE_2D)
        return Dump2DImage(&streamImage, true, zoneName);

    if (streamImage.mapType == game::MAPTYPE_CUBE)
        return DumpCubeImage(&streamImage, true, zoneName);

    return false;
}

bool TryReadStreamPartPixels(const game::GfxImage *image, uint32_t imagePartIndex, std::vector<uint8_t> *pixelData)
{
    if (imagePartIndex >= 4u || pixelData == nullptr)
        return false;

    const game::GfxSubImageStream *sources = GetImageStreamSources(image);
    if (sources == nullptr)
        return false;

    const game::GfxImageStreamData &streamData = image->streams[imagePartIndex];
    const uint32_t expectedSize = streamData.pixelSize & STREAM_PIXEL_SIZE_MASK;
    if (expectedSize == 0)
        return false;

    const game::GfxSubImageStream &source = sources[imagePartIndex];
    if (source.fileOffsetEnd <= source.fileOffset)
        return false;

    const uint32_t compressedSize = source.fileOffsetEnd - source.fileOffset;
    if (compressedSize > MAX_STREAM_COMPRESSED_SIZE)
        return false;

    std::string imageFilePath;
    if (!GetImageFilePath(source, &imageFilePath))
        return false;

    std::vector<uint8_t> compressedData;
    if (!ReadFileRange(imageFilePath, source.fileOffset, compressedSize, &compressedData))
        return false;

    return InflateImageStream(compressedData, expectedSize, pixelData);
}

bool TryDumpStreamPartEager(const game::GfxImage *image, uint32_t imagePartIndex, const char *zoneName)
{
    if (imagePartIndex >= 4u)
        return false;

    const game::GfxImageStreamData &streamData = image->streams[imagePartIndex];
    if ((streamData.pixelSize & STREAM_PIXEL_SIZE_MASK) == 0)
        return false;

    std::vector<uint8_t> pixelData;
    if (!TryReadStreamPartPixels(image, imagePartIndex, &pixelData))
        return false;

    return DumpStreamPartFromData(image, imagePartIndex, zoneName, pixelData);
}

bool TryDumpStreamedImageEager(const game::GfxImage *image, const char *zoneName)
{
    bool usedParts[4] = {false, false, false, false};

    for (uint32_t attempt = 0; attempt < 4u; ++attempt)
    {
        uint32_t bestPart = 4u;
        uint32_t bestArea = 0;

        for (uint32_t imagePartIndex = 0; imagePartIndex < 4u; ++imagePartIndex)
        {
            if (usedParts[imagePartIndex])
                continue;

            const game::GfxImageStreamData &streamData = image->streams[imagePartIndex];
            if ((streamData.pixelSize & STREAM_PIXEL_SIZE_MASK) == 0)
                continue;

            const uint32_t imageArea =
                static_cast<uint32_t>(streamData.width) * static_cast<uint32_t>(streamData.height);
            if (bestPart == 4u || imageArea > bestArea)
            {
                bestPart = imagePartIndex;
                bestArea = imageArea;
            }
        }

        if (bestPart == 4u)
            break;

        usedParts[bestPart] = true;
        if (TryDumpStreamPartEager(image, bestPart, zoneName))
            return true;
    }

    return false;
}

bool Image_Dump(game::GfxImage *image, const char *zoneName)
{
    if (ImageHasStreamedParts(image))
        return TryDumpStreamedImageEager(image, zoneName);

    if (image->mapType == game::MAPTYPE_2D)
        return Dump2DImage(image, false, zoneName);

    if (image->mapType == game::MAPTYPE_CUBE)
        return DumpCubeImage(image, false, zoneName);

    return false;
}

void RegisterDvars()
{
    dump_assets = game::Dvar_RegisterBool("dump_assets", Config::dump_assets, 0, "Dump assets as they are loaded.");
}

bool Image_Replace_2D(game::GfxImage *image, const DDSImage &ddsImage, DdsDataFile &ddsFile, uint32_t ddsDataSize)
{
    if (image->mapType != game::MAPTYPE_2D)
    {
        PrintImageError("image '%s' is not a 2D map\n", image->name);
        return false;
    }

    const D3DBaseTexture *texture = &image->texture.basemap;
    const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(texture->Format.DataFormat);
    const uint32_t levelCount = image::xenos_texture::GetTextureLevelCount(texture);
    const uint32_t mipTailBaseLevel = texture->Format.PackedMips != 0
                                          ? image::xenos_texture::GetMipTailBaseLevel(image->width, image->height)
                                          : levelCount;
    if (!ValidateResidentMipCount(image, ddsImage, levelCount))
        return false;
    if (!ValidateDDSDataSize(image, ddsImage.header, ddsDataSize, format, levelCount, 1u))
        return false;

    const uint32_t nonPackedLevelCount = max(1u, min(levelCount, mipTailBaseLevel));
    unsigned char *baseData = image::xenos_texture::GetTextureBase(texture, image->pixels);
    unsigned char *mipData =
        image::xenos_texture::GetTextureMipBase(texture, baseData, image->width, image->height, format, 1u);

    size_t requiredDDSSize = 0;
    size_t requiredTextureBytes = 0;
    if (!Validate2DReplacementData(image, format, ddsDataSize, nonPackedLevelCount, &requiredDDSSize,
                                   &requiredTextureBytes))
    {
        if (requiredDDSSize == 0)
        {
            PrintImageError("image '%s' has unsupported replacement format %u\n", image->name,
                            static_cast<uint32_t>(format));
        }
        else if (ddsDataSize < requiredDDSSize)
        {
            PrintImageError("image '%s' DDS data is too small: have=%u need=%u for %u mip levels\n", image->name,
                            static_cast<unsigned int>(ddsDataSize), static_cast<unsigned int>(requiredDDSSize),
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

    if (baseData == nullptr || mipData == nullptr)
    {
        PrintImageError("image '%s' has no valid texture memory\n", image->name);
        return false;
    }

    uint32_t ddsOffset = 0;

    for (uint32_t mipLevel = 0; mipLevel < nonPackedLevelCount; ++mipLevel)
    {
        const uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(image->width, mipLevel, format);
        const uint32_t ddsMipLevelSize =
            image::xenos_texture::CalculateLinearLevelSize(image->width, image->height, mipLevel, format);
        const uint32_t tiledMipLevelSize = image::xenos_texture::CalculateTiledLevelSize(
            image->width, image->height, mipLevel, format, texture->Format.Pitch);

        if (ddsMipLevelSize == 0 || tiledMipLevelSize == 0 || rowPitch == 0)
        {
            PrintImageError("unsupported format %u for image '%s' mip level %u\n", texture->Format.DataFormat,
                            image->name, mipLevel);
            return false;
        }

        if (static_cast<size_t>(ddsOffset) + ddsMipLevelSize > ddsDataSize)
        {
            PrintImageError("image '%s' mip level %u exceeds DDS data size\n", image->name, mipLevel);
            return false;
        }

        unsigned char *destination = baseData;
        if (mipLevel > 0)
        {
            destination = mipData + image::xenos_texture::CalculateMipLevelOffset(image->width, image->height, mipLevel,
                                                                                  format, 1u);
        }

        if (!TileDdsLevelToTexture(ddsFile, ddsOffset, ddsMipLevelSize, image->width, image->height, mipLevel,
                                   image->width, image->height, mipLevel, format, texture->Format.Pitch, destination,
                                   tiledMipLevelSize, static_cast<GPUENDIAN>(texture->Format.Endian)))
        {
            PrintImageError("failed to tile image '%s' mip level %u\n", image->name, mipLevel);
            return false;
        }

        ddsOffset += ddsMipLevelSize;
    }

    return true;
}

bool Image_Replace_Cube(game::GfxImage *image, const DDSImage &ddsImage, DdsDataFile &ddsFile, uint32_t ddsDataSize)
{
    if (image->mapType != game::MAPTYPE_CUBE)
    {
        PrintImageError("image '%s' is not a cube map\n", image->name);
        return false;
    }

    const D3DBaseTexture *texture = &image->texture.basemap;
    const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(texture->Format.DataFormat);
    const uint32_t levelCount = image::xenos_texture::GetTextureLevelCount(texture);
    const uint32_t faceSize = image::xenos_texture::CalculateLinearLevelSize(image->width, image->height, 0u, format);
    const uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(image->width, 0u, format);
    const uint32_t tiledFaceSize =
        image::xenos_texture::CalculateTiledLevelSize(image->width, image->height, 0u, format, texture->Format.Pitch);
    const uint32_t tiledBaseSize = image::xenos_texture::CalculateBaseSize(texture, image->width, image->height, 6u);
    unsigned char *baseData = image::xenos_texture::GetTextureBase(texture, image->pixels);

    if (faceSize == 0 || rowPitch == 0 || tiledFaceSize == 0 || tiledBaseSize < tiledFaceSize * 6u)
    {
        PrintImageError("image '%s' has unsupported cube format %u\n", image->name, static_cast<uint32_t>(format));
        return false;
    }

    if (baseData == nullptr)
    {
        PrintImageError("image '%s' has no valid cube texture memory\n", image->name);
        return false;
    }

    if (levelCount != 1u || ddsImage.GetMipCount() != 1u)
    {
        PrintImageError("image '%s' cube replacement must be base-level only: imageMips=%u ddsMips=%u\n", image->name,
                        levelCount, ddsImage.GetMipCount());
        return false;
    }

    if (!ValidateDDSDataSize(image, ddsImage.header, ddsDataSize, format, 1u, 6u))
        return false;

    size_t requiredDDSSize = 0;
    if (!ValidateCubeReplacementData(image, ddsDataSize, format, faceSize, tiledBaseSize, &requiredDDSSize))
    {
        if (ddsDataSize < requiredDDSSize)
        {
            PrintImageError("image '%s' DDS is too small for 6 cube faces: have=%u need=%u\n", image->name,
                            static_cast<unsigned int>(ddsDataSize), static_cast<unsigned int>(requiredDDSSize));
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
        const uint32_t faceOffset = faceIndex * faceSize;
        unsigned char *faceDestination = baseData + (faceIndex * tiledFaceSize);

        if (!TileDdsLevelToTexture(ddsFile, faceOffset, faceSize, image->width, image->height, 0u, image->width,
                                   image->height, 0u, format, texture->Format.Pitch, faceDestination, tiledFaceSize,
                                   static_cast<GPUENDIAN>(texture->Format.Endian)))
        {
            PrintImageError("failed to tile cube image '%s' face %u\n", image->name, faceIndex);
            return false;
        }
    }

    return true;
}

bool ValidateReplacementShape(const game::GfxImage *image, const DDSImage &ddsImage, uint32_t ddsDataSize)
{
    const bool ddsIsCubemap = ddsImage.IsCubemap();

    if (image->mapType == game::MAPTYPE_2D && ddsIsCubemap)
    {
        PrintImageError("image '%s' is 2D but replacement DDS is a cubemap\n", image->name);
        return false;
    }

    if (image->mapType == game::MAPTYPE_CUBE && !ddsIsCubemap)
    {
        GPUTEXTUREFORMAT ddsFormat;
        if (!ddsImage.GetGpuFormat(&ddsFormat))
            return false;

        const uint32_t faceSize = image::xenos_texture::CalculateLinearLevelSize(
            ddsImage.header.dwWidth, ddsImage.header.dwHeight, 0u, ddsFormat);
        if (faceSize == 0 || ddsDataSize < static_cast<size_t>(faceSize) * 6u)
        {
            PrintImageError("image '%s' is a cubemap but replacement DDS is not a valid 6-face cubemap\n", image->name);
            return false;
        }
    }

    return true;
}

void Image_Replace(game::GfxImage *image)
{
    if (image == nullptr || image->name == nullptr)
        return;

    const std::string replacementPath = GetReplacementPath(image->name);
    if (!ImageFileExists(replacementPath))
        return;

    if (ImageHasStreamedParts(image))
        return;

    if (image->pixels == nullptr || image->cardMemory.platform[0] <= 0)
    {
        PrintImageError("image '%s' replacement exists but resident texture memory is not available\n", image->name);
        return;
    }

    DDSImage ddsImage;
    uint32_t ddsDataSize = 0;
    if (!ReadDDSHeader(replacementPath, &ddsImage, &ddsDataSize))
    {
        PrintImageError("failed to load DDS header for image '%s': %s\n", image->name, replacementPath.c_str());
        return;
    }

    GPUTEXTUREFORMAT ddsFormat;
    if (!ValidateDDSHeaderFields(image, ddsImage, &ddsFormat))
        return;

    if (image->width != ddsImage.header.dwWidth || image->height != ddsImage.header.dwHeight)
    {
        PrintImageError("image '%s' dimensions do not match DDS: image=%ux%u dds=%ux%u\n", image->name, image->width,
                        image->height, ddsImage.header.dwWidth, ddsImage.header.dwHeight);
        return;
    }

    if (!ValidateReplacementShape(image, ddsImage, ddsDataSize))
        return;

    DdsDataFile ddsFile(replacementPath, ddsDataSize);
    if (!ddsFile.IsValid())
    {
        PrintImageError("failed to open DDS data for image '%s': %s error=0x%08X\n", image->name,
                        replacementPath.c_str(), GetLastError());
        return;
    }

    bool replaced = false;
    if (image->mapType == game::MAPTYPE_2D)
        replaced = Image_Replace_2D(image, ddsImage, ddsFile, ddsDataSize);
    else if (image->mapType == game::MAPTYPE_CUBE)
        replaced = Image_Replace_Cube(image, ddsImage, ddsFile, ddsDataSize);
    else
        PrintImageError("image '%s' is not a 2D or cube map\n", image->name);

    if (replaced)
        PrintImageInfo("replaced image '%s' (resident)\n", image->name);
}

bool FindMipLevelForDimensions(const DDSImage &ddsImage, uint32_t width, uint32_t height, uint32_t *mipLevel)
{
    const uint32_t ddsMipCount = ddsImage.GetMipCount();

    for (uint32_t currentMip = 0; currentMip < ddsMipCount; ++currentMip)
    {
        if (image::GetMipDimension(ddsImage.header.dwWidth, currentMip) == width &&
            image::GetMipDimension(ddsImage.header.dwHeight, currentMip) == height)
        {
            *mipLevel = currentMip;
            return true;
        }
    }

    return false;
}

bool ValidateStreamReplacementData(const game::GfxImage *image, const DDSImage &ddsImage, size_t ddsDataSize,
                                   GPUTEXTUREFORMAT format, uint32_t startMipLevel, uint32_t levelCount,
                                   uint32_t basePitch, size_t *requiredDDSSize, size_t *requiredTextureBytes)
{
    *requiredDDSSize = image::CalculateRequiredLinearDataSize(ddsImage.header.dwWidth, ddsImage.header.dwHeight, format,
                                                              startMipLevel, levelCount, 1u);
    if (*requiredDDSSize == 0)
        return false;

    const uint32_t ddsOffset =
        image::CalculateDdsMipOffset(ddsImage.header.dwWidth, ddsImage.header.dwHeight, format, startMipLevel);
    if (static_cast<size_t>(ddsOffset) + *requiredDDSSize > ddsDataSize)
        return false;

    *requiredTextureBytes = 0;
    for (uint32_t localMipLevel = 0; localMipLevel < levelCount; ++localMipLevel)
    {
        const uint32_t levelSize = image::xenos_texture::CalculateTiledLevelSize(image->width, image->height,
                                                                                 localMipLevel, format, basePitch);
        if (levelSize == 0)
            return false;

        *requiredTextureBytes += levelSize;
    }

    const int cardMemory = image->cardMemory.platform[0];
    if (cardMemory <= 0 || *requiredTextureBytes > static_cast<size_t>(cardMemory))
        return false;

    return true;
}

bool Image_Replace_StreamCubePart(game::GfxImage *image, DdsDataFile &ddsFile, const DDSImage &ddsImage,
                                  size_t ddsDataSize, GPUTEXTUREFORMAT ddsFormat, uint32_t imagePartIndex)
{
    if (imagePartIndex != 0u)
    {
        PrintImageError("streamed cube image '%s' has unsupported part %u\n", image->name, imagePartIndex);
        return false;
    }

    if (image->width != ddsImage.header.dwWidth || image->height != ddsImage.header.dwHeight)
    {
        PrintImageError("streamed cube image '%s' dimensions do not match DDS: image=%ux%u dds=%ux%u\n", image->name,
                        image->width, image->height, ddsImage.header.dwWidth, ddsImage.header.dwHeight);
        return false;
    }

    const uint32_t ddsMipCount = ddsImage.GetMipCount();
    if (image->levelCount != 1u || ddsMipCount != 1u)
    {
        PrintImageError("streamed cube image '%s' must be base-level only: imageMips=%u ddsMips=%u\n", image->name,
                        static_cast<uint32_t>(image->levelCount), ddsMipCount);
        return false;
    }

    if (!ValidateDDSDataSize(image, ddsImage.header, ddsDataSize, ddsFormat, 1u, 6u))
        return false;

    const uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(image->width, 0u, ddsFormat);
    const uint32_t faceSize =
        image::xenos_texture::CalculateLinearLevelSize(image->width, image->height, 0u, ddsFormat);
    const uint32_t tiledFaceSize =
        image::xenos_texture::CalculateTiledLevelSize(image->width, image->height, 0u, ddsFormat, 0u);
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
        const uint32_t faceOffset = faceIndex * faceSize;
        unsigned char *faceDestination = image->pixels + static_cast<size_t>(faceIndex) * tiledFaceSize;

        if (!TileDdsLevelToTexture(ddsFile, faceOffset, faceSize, image->width, image->height, 0u, image->width,
                                   image->height, 0u, ddsFormat, 0u, faceDestination, tiledFaceSize,
                                   static_cast<GPUENDIAN>(image->texture.basemap.Format.Endian)))
        {
            PrintImageError("failed to tile streamed cube image '%s' face %u\n", image->name, faceIndex);
            return false;
        }
    }

    return true;
}

bool Image_Replace_StreamPart(game::GfxImage *image, DdsDataFile &ddsFile, const DDSImage &ddsImage,
                              size_t ddsDataSize, uint32_t imagePartIndex)
{
    if (image == nullptr || image->name == nullptr || imagePartIndex >= 4u)
        return false;

    if (image->pixels == nullptr)
    {
        PrintImageError("streamed image '%s' part %u replacement exists but pixel memory is not available\n",
                        image->name, imagePartIndex);
        return false;
    }

    GPUTEXTUREFORMAT ddsFormat;
    if (!ValidateDDSHeaderFields(image, ddsImage, &ddsFormat))
        return false;

    if (image->mapType == game::MAPTYPE_CUBE)
        return Image_Replace_StreamCubePart(image, ddsFile, ddsImage, ddsDataSize, ddsFormat, imagePartIndex);

    if (image->mapType != game::MAPTYPE_2D)
    {
        PrintImageError("streamed image '%s' is not a supported 2D or cube map\n", image->name);
        return false;
    }

    if (ddsImage.IsCubemap())
    {
        PrintImageError("streamed image '%s' is 2D but replacement DDS is a cubemap\n", image->name);
        return false;
    }

    const uint32_t ddsMipCount = ddsImage.GetMipCount();
    if (!ValidateDDSDataSize(image, ddsImage.header, ddsDataSize, ddsFormat, ddsMipCount, 1u))
        return false;

    uint32_t startMipLevel = 0;
    if (!FindMipLevelForDimensions(ddsImage, image->width, image->height, &startMipLevel))
    {
        PrintImageError("streamed image '%s' part %u dimensions do not exist in DDS: part=%ux%u dds=%ux%u\n",
                        image->name, imagePartIndex, image->width, image->height, ddsImage.header.dwWidth,
                        ddsImage.header.dwHeight);
        return false;
    }

    const game::GfxImageStreamData &streamData = image->streams[imagePartIndex];
    uint32_t levelCount = streamData.pixelSize >> 26;
    if (levelCount == 0)
        levelCount = max(1u, static_cast<uint32_t>(image->levelCount));

    const D3DBaseTexture *texture = &image->texture.basemap;
    const uint32_t textureLevelCount = image::xenos_texture::GetTextureLevelCount(texture);
    levelCount = max(1u, min(levelCount, textureLevelCount));

    const uint32_t mipTailBaseLevel = texture->Format.PackedMips != 0
                                          ? image::xenos_texture::GetMipTailBaseLevel(image->width, image->height)
                                          : levelCount;
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
    if (!ValidateStreamReplacementData(image, ddsImage, ddsDataSize, ddsFormat, startMipLevel, replaceLevelCount,
                                       streamBasePitch, &requiredDDSSize, &requiredTextureBytes))
    {
        PrintImageError("streamed image '%s' part %u replacement size is invalid: ddsNeed=%u textureNeed=%u card=%u\n",
                        image->name, imagePartIndex, static_cast<unsigned int>(requiredDDSSize),
                        static_cast<unsigned int>(requiredTextureBytes),
                        static_cast<unsigned int>(image->cardMemory.platform[0]));
        return false;
    }

    uint32_t ddsOffset =
        image::CalculateDdsMipOffset(ddsImage.header.dwWidth, ddsImage.header.dwHeight, ddsFormat, startMipLevel);
    size_t destinationOffset = 0;

    for (uint32_t localMipLevel = 0; localMipLevel < replaceLevelCount; ++localMipLevel)
    {
        const uint32_t globalMipLevel = startMipLevel + localMipLevel;
        const uint32_t rowPitch =
            image::xenos_texture::CalculateLinearRowPitch(ddsImage.header.dwWidth, globalMipLevel, ddsFormat);
        const uint32_t ddsMipLevelSize = image::xenos_texture::CalculateLinearLevelSize(
            ddsImage.header.dwWidth, ddsImage.header.dwHeight, globalMipLevel, ddsFormat);
        const uint32_t tiledMipLevelSize = image::xenos_texture::CalculateTiledLevelSize(
            image->width, image->height, localMipLevel, ddsFormat, streamBasePitch);

        if (rowPitch == 0 || ddsMipLevelSize == 0 || tiledMipLevelSize == 0)
        {
            PrintImageError("streamed image '%s' part %u has unsupported format %u at mip %u\n", image->name,
                            imagePartIndex, static_cast<uint32_t>(ddsFormat), localMipLevel);
            return false;
        }

        if (static_cast<size_t>(ddsOffset) + ddsMipLevelSize > ddsDataSize ||
            destinationOffset + tiledMipLevelSize > static_cast<size_t>(image->cardMemory.platform[0]))
        {
            PrintImageError("streamed image '%s' part %u mip %u exceeds source or destination bounds\n", image->name,
                            imagePartIndex, localMipLevel);
            return false;
        }

        unsigned char *destination = image->pixels + destinationOffset;

        if (!TileDdsLevelToTexture(ddsFile, ddsOffset, ddsMipLevelSize, ddsImage.header.dwWidth,
                                   ddsImage.header.dwHeight, globalMipLevel, image->width, image->height,
                                   localMipLevel, ddsFormat, streamBasePitch, destination, tiledMipLevelSize,
                                   static_cast<GPUENDIAN>(image->texture.basemap.Format.Endian)))
        {
            PrintImageError("failed to tile streamed image '%s' part %u mip %u\n", image->name, imagePartIndex,
                            localMipLevel);
            return false;
        }

        ddsOffset += ddsMipLevelSize;
        destinationOffset += tiledMipLevelSize;
    }

    return true;
}

void TryReplaceStreamPart(game::GfxImage *image, uint32_t imagePartIndex)
{
    if (image == nullptr || image->name == nullptr)
        return;

    const std::string replacementPath = GetReplacementPath(image->name);
    const bool replacementExists = ImageFileExists(replacementPath);

    if (imagePartIndex >= 4u)
        return;

    if (!replacementExists)
        return;

    DDSImage ddsImage;
    uint32_t ddsDataSize = 0;
    if (!ReadDDSHeader(replacementPath, &ddsImage, &ddsDataSize))
    {
        PrintImageError("failed to load DDS header for image '%s': %s\n", image->name, replacementPath.c_str());
        return;
    }

    DdsDataFile ddsFile(replacementPath, ddsDataSize);
    if (!ddsFile.IsValid())
    {
        PrintImageError("failed to open DDS data for streamed image '%s': %s error=0x%08X\n", image->name,
                        replacementPath.c_str(), GetLastError());
        return;
    }

    if (Image_Replace_StreamPart(image, ddsFile, ddsImage, ddsDataSize, imagePartIndex))
        PrintImageInfo("replaced image '%s' (streamed part %u)\n", image->name, imagePartIndex);
}

void OnDBLinkXAssetPre(game::XAssetType &type, game::XAssetHeader *header)
{
    if (type == game::ASSET_TYPE_IMAGE && header != nullptr)
        Image_Replace(header->image);
}

void OnDBLinkXAssetPost(game::XAssetEntryPoolEntry *poolEntry)
{
    if (dump_assets == nullptr || !dump_assets->current.enabled || poolEntry == nullptr)
        return;

    const game::XAssetEntry &entry = poolEntry->entry;
    if (entry.asset.type != game::ASSET_TYPE_IMAGE)
        return;

    game::GfxImage *image = entry.asset.header.image;
    Image_Dump(image, GetZoneName(entry.zoneIndex));
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
    Events::OnDvarInit(RegisterDvars);
    Events::OnDBLinkXAssetPre(OnDBLinkXAssetPre);
    Events::OnDBLinkXAssetPost(OnDBLinkXAssetPost);

    ImageCache_InitImage_Detour = Detour(iw4::mp_tu6::ImageCache_InitImage, ImageCache_InitImage_Hook);
    ImageCache_InitImage_Detour.Install();
}

image_loader::~image_loader()
{
    ImageCache_InitImage_Detour.Remove();
    dump_assets = nullptr;
}
