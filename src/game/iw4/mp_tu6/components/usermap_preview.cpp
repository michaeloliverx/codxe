#include "pch.h"
#include "usermap_preview.h"
#include "arenas.h"
#include "usermaps.h"
#include "image/usermap_preview.h"
#include "image/preview_dds.h"

namespace iw4
{
namespace mp_tu6
{
namespace
{
const char *const PREVIEW_MATERIAL = "$codxe_usermap_preview";
GfxImage preview = {};
Material material = {};
MaterialTextureDef texture = {};
Detour Material_RegisterHandle_Detour;

bool AllocateImage()
{
    if (preview.pixels)
        return true;
    UINT size = 0;
    XGSetTextureHeader_Native(image::preview::WIDTH, image::preview::HEIGHT, 1, 0, D3DFMT_DXT1, D3DPOOL_DEFAULT, 0, 0,
                              0, &preview.texture.map, &size, nullptr);
    preview.pixels =
        static_cast<unsigned char *>(XPhysicalAlloc(size, MAXULONG_PTR, 4096, PAGE_READWRITE | PAGE_WRITECOMBINE));
    if (!preview.pixels)
        return false;
    memset(preview.pixels, 0, size);
    XGOffsetResourceAddress_Native(&preview.texture.basemap, preview.pixels);
    preview.format = D3DFMT_DXT1;
    preview.mapType = 3;
    preview.semantic = 2;
    preview.category = 1;
    preview.cardMemory = size;
    preview.width = image::preview::WIDTH;
    preview.height = image::preview::HEIGHT;
    preview.depth = 1;
    preview.levelCount = 1;
    preview.name = PREVIEW_MATERIAL;
    return true;
}

Material *RegisterMaterial(const char *name)
{
    auto original = Material_RegisterHandle_Detour.GetOriginal<Material_RegisterHandle_t>();
    if (!name)
        return original(name);
    if (std::strcmp(name, PREVIEW_MATERIAL) != 0)
    {
        // The private lobby builds preview_<map> directly instead of using
        // getmapimage(). Resolve installed custom maps through the same image.
        if (std::strncmp(name, "preview_mp_", 11) != 0)
            return original(name);
        const char *map = name + std::strlen("preview_");
        if (!Usermaps::EnsurePreview(map))
        {
            std::string value;
            return Arenas::GetField(map, "mapimage", value) ? original("logo_cod2") : original(name);
        }
    }
    Material *base = original("white");
    if (!base || base->textureCount != 1 || !base->textureTable || !preview.pixels)
        return original("logo_cod2");
    // Keep one private image across fastfile unloads. Reacquire the resident UI
    // material's technique and state so a frontend reload cannot leave stale pointers.
    material = *base;
    texture = base->textureTable[0];
    material.info.name = PREVIEW_MATERIAL;
    material.textureTable = &texture;
    texture.u.image = &preview;
    return &material;
}
} // namespace

bool UsermapPreview::Load(const std::string &path)
{
    if (path.empty() || !AllocateImage())
        return false;
    const image::preview::DeviceFunctions backend = {gfxDevice, R_SyncRenderThread, D3D_BlockUntilIdle,
                                                     D3D_InvalidateCaches, nullptr};
    return image::preview::UploadDDS(path, &preview.texture.basemap, preview.pixels, preview.width, preview.height,
                                     backend);
}

UsermapPreview::UsermapPreview()
{
    Material_RegisterHandle_Detour = Detour(Material_RegisterHandle, RegisterMaterial);
    Material_RegisterHandle_Detour.Install();
}

UsermapPreview::~UsermapPreview()
{
    Material_RegisterHandle_Detour.Remove();
    if (preview.pixels)
    {
        R_SyncRenderThread();
        if (*gfxDevice)
            D3D_BlockUntilIdle(*gfxDevice);
        XPhysicalFree(preview.pixels);
        memset(&preview, 0, sizeof(preview));
    }
}
} // namespace mp_tu6
} // namespace iw4
