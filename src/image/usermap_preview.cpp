#include "pch.h"
#include "usermap_preview.h"
#include "preview_dds.h"
#include "xenos_texture.h"

namespace image
{
namespace preview
{
namespace
{
void InvalidateIW3(D3DDevice *device, unsigned char *pixels, unsigned int size, unsigned int (*reserve)(D3DDevice *))
{
    // TU4's SDK inlines the range invalidation command. Its push-buffer fields
    // are at 0x30/0x38; reserve through the game's SDK if the buffer is full.
    unsigned int *fields = reinterpret_cast<unsigned int *>(device);
    unsigned int current = fields[12];
    if (current > fields[14])
        current = reserve(device);
    volatile unsigned int *commands = reinterpret_cast<volatile unsigned int *>(current);
    const unsigned int address = reinterpret_cast<unsigned int>(pixels);
    const unsigned int physical = (((address >> 20) + 512) & 0x1000) + (address & 0x1FFFFFFF);
    const unsigned int first = physical & ~4095u;
    *++commands = 2609;
    *++commands = 0x03000000;
    *++commands = 68143;
    *++commands = ((physical + size + 4095) & ~4095u) - first;
    *++commands = first;
    *++commands = 0xC0043C00;
    *++commands = 3;
    *++commands = 2609;
    *++commands = 0;
    *++commands = 0x80000000;
    *++commands = 8;
    fields[12] = reinterpret_cast<unsigned int>(commands);
}
} // namespace

bool UploadDDS(const std::string &path, D3DBaseTexture *texture, unsigned char *pixels, unsigned int width,
               unsigned int height, const DeviceFunctions &backend)
{
    if (path.empty() || !texture || !pixels || width != WIDTH || height != HEIGHT || !backend.device ||
        !*backend.device || !backend.sync || !backend.waitIdle || (!backend.invalidate && !backend.reserve))
        return false;
    const unsigned int format = texture->Format.DataFormat;
    if (format != GPUTEXTUREFORMAT_DXT1)
        return false;
    HANDLE file = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;
    DWORD high = 0, read = 0;
    const DWORD size = GetFileSize(file, &high);
    std::vector<unsigned char> data;
    bool valid = !high && size == FILE_SIZE;
    if (valid)
    {
        data.resize(size);
        valid = ReadFile(file, data.data(), size, &read, nullptr) && read == size;
    }
    CloseHandle(file);
    const unsigned char *source = nullptr;
    if (!valid || !ReadDDS(data.data(), data.size(), &source, nullptr))
        return false;
    const unsigned int pitch = texture->Format.Pitch;
    const unsigned int tiledSize = xenos_texture::CalculateTiledLevelSize(width, height, 0, format, pitch);
    unsigned char *destination = xenos_texture::GetTextureBase(texture, pixels);
    if (!tiledSize || !destination)
        return false;
    std::vector<unsigned char> linear(source, source + PIXEL_SIZE);
    xenos_texture::ApplyGpuEndian(linear.data(), linear.size(), static_cast<GPUENDIAN>(texture->Format.Endian));
    std::vector<unsigned char> tiled(tiledSize);
    if (!xenos_texture::TileTextureLevel(width, height, 0, format, pitch, tiled.data(), tiled.size(), linear.data(),
                                         linear.size(), xenos_texture::CalculateLinearRowPitch(width, 0, format)))
        return false;
    backend.sync();
    backend.waitIdle(*backend.device);
    XMemCpyStreaming_WriteCombined(destination, tiled.data(), tiled.size());
    __sync();
    if (backend.invalidate)
        backend.invalidate(*backend.device, destination, tiledSize);
    else
        InvalidateIW3(*backend.device, destination, tiledSize, backend.reserve);
    return true;
}
} // namespace preview
} // namespace image
