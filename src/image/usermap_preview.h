#pragma once

#include "pch.h"

namespace image
{
namespace preview
{
struct DeviceFunctions
{
    D3DDevice **device;
    void (*sync)();
    void (*waitIdle)(D3DDevice *);
    void (*invalidate)(D3DDevice *, void *, unsigned int);
    unsigned int (*reserve)(D3DDevice *);
};

bool UploadDDS(const std::string &path, D3DBaseTexture *texture, unsigned char *pixels, unsigned int width,
               unsigned int height, const DeviceFunctions &backend);
} // namespace preview
} // namespace image
