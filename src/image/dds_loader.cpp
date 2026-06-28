#include "pch.h"
#include "image/dds_loader.h"
#include "utils/endian.h"

#ifndef INVALID_FILE_SIZE
#define INVALID_FILE_SIZE ((DWORD)-1)
#endif

namespace image
{
namespace
{
const size_t DDS_FILE_HEADER_SIZE = sizeof(uint32_t) + sizeof(DDS_HEADER);
const DWORD MAX_DDS_FILE_SIZE = 64u * 1024u * 1024u;

struct ScopedFileHandle
{
    HANDLE handle;

    explicit ScopedFileHandle(HANDLE file) : handle(file)
    {
    }

    ~ScopedFileHandle()
    {
        if (handle != INVALID_HANDLE_VALUE)
            CloseHandle(handle);
    }

    bool IsValid() const
    {
        return handle != INVALID_HANDLE_VALUE;
    }
};

bool ReadExact(HANDLE file, const std::string &path, void *buffer, DWORD size, const char *label)
{
    DWORD bytesRead = 0;
    if (!ReadFile(file, buffer, size, &bytesRead, nullptr) || bytesRead != size)
    {
        DbgPrint("[codxe][DDS] failed to read %s from '%s': expected=%u actual=%u error=0x%08X\n", label,
                 path.c_str(), size, bytesRead, GetLastError());
        return false;
    }

    return true;
}

void SwapDDSHeaderEndian(DDS_HEADER &header)
{
    utils::endian::ByteSwap(header.dwSize);
    utils::endian::ByteSwap(header.dwFlags);
    utils::endian::ByteSwap(header.dwHeight);
    utils::endian::ByteSwap(header.dwWidth);
    utils::endian::ByteSwap(header.dwPitchOrLinearSize);
    utils::endian::ByteSwap(header.dwDepth);
    utils::endian::ByteSwap(header.dwMipMapCount);

    for (int i = 0; i < 11; i++)
        utils::endian::ByteSwap(header.dwReserved1[i]);

    utils::endian::ByteSwap(header.ddspf.dwSize);
    utils::endian::ByteSwap(header.ddspf.dwFlags);
    utils::endian::ByteSwap(header.ddspf.dwFourCC);
    utils::endian::ByteSwap(header.ddspf.dwRGBBitCount);
    utils::endian::ByteSwap(header.ddspf.dwRBitMask);
    utils::endian::ByteSwap(header.ddspf.dwGBitMask);
    utils::endian::ByteSwap(header.ddspf.dwBBitMask);
    utils::endian::ByteSwap(header.ddspf.dwABitMask);

    utils::endian::ByteSwap(header.dwCaps);
    utils::endian::ByteSwap(header.dwCaps2);
    utils::endian::ByteSwap(header.dwCaps3);
    utils::endian::ByteSwap(header.dwCaps4);
    utils::endian::ByteSwap(header.dwReserved2);
}

bool IsValidDdsImage(const DdsImage &image)
{
    return image.header.dwSize == DDS_HEADER_SIZE && image.header.ddspf.dwSize == DDS_PIXEL_FORMAT_SIZE &&
           !image.data.empty();
}

void ResetDdsImage(DdsImage *image)
{
    if (image == nullptr)
        return;

    ZeroMemory(&image->header, sizeof(image->header));
    DdsByteVector().swap(image->data);
}
} // namespace

bool DdsImage::IsCubemap() const
{
    const uint32_t cubemapFaces = DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_NEGATIVEX | DDSCAPS2_CUBEMAP_POSITIVEY |
                                  DDSCAPS2_CUBEMAP_NEGATIVEY | DDSCAPS2_CUBEMAP_POSITIVEZ | DDSCAPS2_CUBEMAP_NEGATIVEZ;

    return (header.dwCaps2 & DDSCAPS2_CUBEMAP) != 0 || (header.dwCaps2 & cubemapFaces) == cubemapFaces;
}

uint32_t DdsImage::GetMipCount() const
{
    return max(1u, static_cast<uint32_t>(header.dwMipMapCount));
}

bool DdsImage::GetGpuFormat(GPUTEXTUREFORMAT *format) const
{
    if (format == nullptr)
        return false;

    if ((header.ddspf.dwFlags & DDPF_FOURCC) != 0)
    {
        switch (header.ddspf.dwFourCC)
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

    if ((header.ddspf.dwFlags & DDPF_LUMINANCE) != 0)
    {
        if (header.ddspf.dwRGBBitCount == 8 && header.ddspf.dwRBitMask == 0x000000FF)
        {
            *format = GPUTEXTUREFORMAT_8;
            return true;
        }

        if ((header.ddspf.dwFlags & DDPF_ALPHAPIXELS) != 0 && header.ddspf.dwRGBBitCount == 16 &&
            header.ddspf.dwRBitMask == 0x000000FF && header.ddspf.dwABitMask == 0x0000FF00)
        {
            *format = GPUTEXTUREFORMAT_8_8;
            return true;
        }
    }

    if ((header.ddspf.dwFlags & DDPF_RGB) != 0 && (header.ddspf.dwFlags & DDPF_ALPHAPIXELS) != 0 &&
        header.ddspf.dwRGBBitCount == 32 && header.ddspf.dwRBitMask == 0x00FF0000 &&
        header.ddspf.dwGBitMask == 0x0000FF00 && header.ddspf.dwBBitMask == 0x000000FF &&
        header.ddspf.dwABitMask == 0xFF000000)
    {
        *format = GPUTEXTUREFORMAT_8_8_8_8;
        return true;
    }

    return false;
}

bool LoadDdsHeaderFromFile(const std::string &path, DDS_HEADER *outHeader, uint32_t *outDataSize)
{
    if (outHeader == nullptr)
        return false;

    ZeroMemory(outHeader, sizeof(*outHeader));
    if (outDataSize != nullptr)
        *outDataSize = 0;

    ScopedFileHandle file(CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
    if (!file.IsValid())
    {
        DbgPrint("[codxe][DDS] failed to open '%s': error=0x%08X\n", path.c_str(), GetLastError());
        return false;
    }

    const DWORD fileSize = GetFileSize(file.handle, nullptr);
    if (fileSize == INVALID_FILE_SIZE)
    {
        DbgPrint("[codxe][DDS] failed to get size for '%s': error=0x%08X\n", path.c_str(), GetLastError());
        return false;
    }

    if (fileSize < DDS_FILE_HEADER_SIZE)
    {
        DbgPrint("[codxe][DDS] file too small '%s': size=%u header=%u\n", path.c_str(), fileSize,
                 static_cast<unsigned int>(DDS_FILE_HEADER_SIZE));
        return false;
    }

    if (fileSize > MAX_DDS_FILE_SIZE)
    {
        DbgPrint("[codxe][DDS] file too large '%s': size=%u max=%u\n", path.c_str(), fileSize, MAX_DDS_FILE_SIZE);
        return false;
    }

    uint32_t magic = 0;
    if (!ReadExact(file.handle, path, &magic, sizeof(magic), "magic"))
        return false;

    utils::endian::ByteSwap(magic);
    if (magic != DDS_MAGIC)
    {
        DbgPrint("[codxe][DDS] invalid magic '%s': 0x%08X\n", path.c_str(), magic);
        return false;
    }

    if (!ReadExact(file.handle, path, outHeader, sizeof(DDS_HEADER), "header"))
        return false;

    SwapDDSHeaderEndian(*outHeader);

    if (outDataSize != nullptr)
        *outDataSize = fileSize - static_cast<DWORD>(DDS_FILE_HEADER_SIZE);

    return true;
}

bool LoadDdsDataRangeFromFile(const std::string &path, uint32_t dataOffset, void *buffer, uint32_t size)
{
    if (buffer == nullptr || size == 0)
        return false;

    ScopedFileHandle file(CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
    if (!file.IsValid())
    {
        DbgPrint("[codxe][DDS] failed to open '%s': error=0x%08X\n", path.c_str(), GetLastError());
        return false;
    }

    const DWORD fileSize = GetFileSize(file.handle, nullptr);
    if (fileSize == INVALID_FILE_SIZE)
    {
        DbgPrint("[codxe][DDS] failed to get size for '%s': error=0x%08X\n", path.c_str(), GetLastError());
        return false;
    }

    if (fileSize < DDS_FILE_HEADER_SIZE || dataOffset > fileSize - DDS_FILE_HEADER_SIZE ||
        size > fileSize - DDS_FILE_HEADER_SIZE - dataOffset)
    {
        DbgPrint("[codxe][DDS] range outside file '%s': dataOffset=%u size=%u fileSize=%u\n", path.c_str(),
                 dataOffset, size, fileSize);
        return false;
    }

    const DWORD fileOffset = static_cast<DWORD>(DDS_FILE_HEADER_SIZE) + dataOffset;
    SetLastError(NO_ERROR);
    const DWORD seekResult = SetFilePointer(file.handle, fileOffset, nullptr, FILE_BEGIN);
    if (seekResult == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
    {
        DbgPrint("[codxe][DDS] failed to seek '%s': offset=%u error=0x%08X\n", path.c_str(), fileOffset,
                 GetLastError());
        return false;
    }

    return ReadExact(file.handle, path, buffer, size, "data range");
}

bool LoadDdsFromFile(const std::string &path, DdsImage *out)
{
    if (out == nullptr)
        return false;

    const char *stage = "init";
    DWORD dataSize = 0;

    try
    {
        ResetDdsImage(out);

        stage = "open";
        ScopedFileHandle file(CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
        if (!file.IsValid())
        {
            DbgPrint("[codxe][DDS] failed to open '%s': error=0x%08X\n", path.c_str(), GetLastError());
            return false;
        }

        stage = "get size";
        const DWORD fileSize = GetFileSize(file.handle, nullptr);
        if (fileSize == INVALID_FILE_SIZE)
        {
            DbgPrint("[codxe][DDS] failed to get size for '%s': error=0x%08X\n", path.c_str(), GetLastError());
            return false;
        }

        if (fileSize < DDS_FILE_HEADER_SIZE)
        {
            DbgPrint("[codxe][DDS] file too small '%s': size=%u header=%u\n", path.c_str(), fileSize,
                     static_cast<unsigned int>(DDS_FILE_HEADER_SIZE));
            return false;
        }

        if (fileSize > MAX_DDS_FILE_SIZE)
        {
            DbgPrint("[codxe][DDS] file too large '%s': size=%u max=%u\n", path.c_str(), fileSize,
                     MAX_DDS_FILE_SIZE);
            return false;
        }

        stage = "read magic";
        uint32_t magic = 0;
        if (!ReadExact(file.handle, path, &magic, sizeof(magic), "magic"))
            return false;

        utils::endian::ByteSwap(magic);
        if (magic != DDS_MAGIC)
        {
            DbgPrint("[codxe][DDS] invalid magic '%s': 0x%08X\n", path.c_str(), magic);
            return false;
        }

        stage = "read header";
        if (!ReadExact(file.handle, path, &out->header, sizeof(DDS_HEADER), "header"))
            return false;

        SwapDDSHeaderEndian(out->header);

        dataSize = fileSize - static_cast<DWORD>(DDS_FILE_HEADER_SIZE);
        stage = "allocate data";
        out->data.resize(dataSize);

        stage = "read data";
        if (dataSize > 0 && !ReadExact(file.handle, path, &out->data[0], dataSize, "data"))
        {
            ResetDdsImage(out);
            return false;
        }

        return IsValidDdsImage(*out);
    }
    catch (const std::bad_alloc &)
    {
        DbgPrint("[codxe][DDS] allocation failed while loading '%s': stage=%s dataSize=%u error=0x%08X\n",
                 path.c_str(), stage, dataSize, GetLastError());
        ResetDdsImage(out);
        return false;
    }
    catch (...)
    {
        DbgPrint("[codxe][DDS] exception while loading '%s': stage=%s dataSize=%u error=0x%08X\n", path.c_str(),
                 stage, dataSize, GetLastError());
        ResetDdsImage(out);
        return false;
    }
}

DdsImage LoadDdsFromFile(const std::string &path)
{
    DdsImage image;
    LoadDdsFromFile(path, &image);
    return image;
}

uint32_t GetMipDimension(uint32_t dimension, uint32_t mipLevel)
{
    return max(1u, dimension >> mipLevel);
}
} // namespace image
