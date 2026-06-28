#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <vector>

namespace image
{
template <typename T> class VirtualAllocAllocator
{
  public:
    typedef T value_type;
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T &reference;
    typedef const T &const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    template <typename U> struct rebind
    {
        typedef VirtualAllocAllocator<U> other;
    };

    VirtualAllocAllocator()
    {
    }

    template <typename U> VirtualAllocAllocator(const VirtualAllocAllocator<U> &)
    {
    }

    pointer allocate(size_type count, const void * = nullptr)
    {
        if (count == 0)
            return nullptr;

        if (count > max_size())
            throw std::bad_alloc();

        void *memory = VirtualAlloc(nullptr, count * sizeof(T), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (memory == nullptr)
            throw std::bad_alloc();

        return static_cast<pointer>(memory);
    }

    void deallocate(pointer memory, size_type)
    {
        if (memory != nullptr)
            VirtualFree(memory, 0, MEM_RELEASE);
    }

    void construct(pointer memory, const_reference value)
    {
        new (memory) T(value);
    }

    void destroy(pointer memory)
    {
        memory->~T();
    }

    size_type max_size() const
    {
        return static_cast<size_type>(-1) / sizeof(T);
    }
};

template <typename T, typename U>
bool operator==(const VirtualAllocAllocator<T> &, const VirtualAllocAllocator<U> &)
{
    return true;
}

template <typename T, typename U>
bool operator!=(const VirtualAllocAllocator<T> &, const VirtualAllocAllocator<U> &)
{
    return false;
}

typedef std::vector<uint8_t, VirtualAllocAllocator<uint8_t>> DdsByteVector;

enum DDS_CONSTANTS
{
    DDS_MAGIC = MAKEFOURCC('D', 'D', 'S', ' '),
    DDS_HEADER_SIZE = 124u,
    DDS_PIXEL_FORMAT_SIZE = 32u,
};

enum DDS_FOURCC
{
    DXT1_FOURCC = MAKEFOURCC('D', 'X', 'T', '1'),
    DXT3_FOURCC = MAKEFOURCC('D', 'X', 'T', '3'),
    DXT5_FOURCC = MAKEFOURCC('D', 'X', 'T', '5'),
    DXN_FOURCC = MAKEFOURCC('A', 'T', 'I', '2'),
};

enum DDP_FLAGS
{
    DDPF_ALPHAPIXELS = 0x1,
    DDPF_ALPHA = 0x2,
    DDPF_FOURCC = 0x4,
    DDPF_RGB = 0x40,
    DDPF_YUV = 0x200,
    DDPF_LUMINANCE = 0x20000,
};

enum DDS_HEADER_FLAGS
{
    DDSD_CAPS = 0x1,
    DDSD_HEIGHT = 0x2,
    DDSD_WIDTH = 0x4,
    DDSD_PITCH = 0x8,
    DDSD_PIXELFORMAT = 0x1000,
    DDSD_MIPMAPCOUNT = 0x20000,
    DDSD_LINEARSIZE = 0x80000,
    DDSD_DEPTH = 0x800000,
};

enum DDS_HEADER_CAPS
{
    DDSCAPS_COMPLEX = 0x8,
    DDSCAPS_TEXTURE = 0x1000,
    DDSCAPS_MIPMAP = 0x400000,
};

enum DDS_HEADER_CAPS2
{
    DDSCAPS2_CUBEMAP = 0x200,
    DDSCAPS2_CUBEMAP_POSITIVEX = 0x400,
    DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800,
    DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000,
    DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000,
    DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000,
    DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000,
    DDSCAPS2_VOLUME = 0x200000,
};

struct DDS_PIXELFORMAT
{
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwFourCC;
    uint32_t dwRGBBitCount;
    uint32_t dwRBitMask;
    uint32_t dwGBitMask;
    uint32_t dwBBitMask;
    uint32_t dwABitMask;
};

static_assert(sizeof(DDS_PIXELFORMAT) == 32, "");

struct DDS_HEADER
{
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwHeight;
    uint32_t dwWidth;
    uint32_t dwPitchOrLinearSize;
    uint32_t dwDepth;
    uint32_t dwMipMapCount;
    uint32_t dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    uint32_t dwCaps;
    uint32_t dwCaps2;
    uint32_t dwCaps3;
    uint32_t dwCaps4;
    uint32_t dwReserved2;
};

static_assert(sizeof(DDS_HEADER) == DDS_HEADER_SIZE, "");

struct DdsImage
{
    DDS_HEADER header;
    DdsByteVector data;

    bool IsCubemap() const;
    uint32_t GetMipCount() const;
    bool GetGpuFormat(GPUTEXTUREFORMAT *format) const;
};
} // namespace image
