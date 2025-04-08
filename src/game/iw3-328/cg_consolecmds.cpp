#pragma warning(push)
#pragma warning(disable : 4480)

#include <xtl.h>
#include <xgraphics.h>
#include <fstream>

#include <algorithm>

#include "..\..\detour.h"
#include "..\..\filesystem.h"
#include "..\..\xboxkrnl.h"

namespace iw3_328
{
    typedef void (*CG_InitConsoleCommands_t)();
    CG_InitConsoleCommands_t CG_InitConsoleCommands = reinterpret_cast<CG_InitConsoleCommands_t>(0x8211F608);

    struct cmd_function_s
    {
        cmd_function_s *next;
        const char *name;
        const char *autoCompleteDir;
        const char *autoCompleteExt;
        void (*function)();
    };

    typedef void (*Cmd_AddCommandInternal_t)(const char *cmdName, void (*function)(), cmd_function_s *allocedCmd);
    Cmd_AddCommandInternal_t Cmd_AddCommandInternal = reinterpret_cast<Cmd_AddCommandInternal_t>(0x821FEA68);

    enum conChannel_t : __int32
    {
        CON_CHANNEL_DONT_FILTER = 0x0,
        CON_CHANNEL_ERROR = 0x1,
        CON_CHANNEL_GAMENOTIFY = 0x2,
        CON_CHANNEL_BOLDGAME = 0x3,
        CON_CHANNEL_SUBTITLE = 0x4,
        CON_CHANNEL_OBITUARY = 0x5,
        CON_CHANNEL_LOGFILEONLY = 0x6,
        CON_CHANNEL_CONSOLEONLY = 0x7,
        CON_CHANNEL_GFX = 0x8,
        CON_CHANNEL_SOUND = 0x9,
        CON_CHANNEL_FILES = 0xA,
        CON_CHANNEL_DEVGUI = 0xB,
        CON_CHANNEL_PROFILE = 0xC,
        CON_CHANNEL_UI = 0xD,
        CON_CHANNEL_CLIENT = 0xE,
        CON_CHANNEL_SERVER = 0xF,
        CON_CHANNEL_SYSTEM = 0x10,
        CON_CHANNEL_PLAYERWEAP = 0x11,
        CON_CHANNEL_AI = 0x12,
        CON_CHANNEL_ANIM = 0x13,
        CON_CHANNEL_PHYS = 0x14,
        CON_CHANNEL_FX = 0x15,
        CON_CHANNEL_LEADERBOARDS = 0x16,
        CON_CHANNEL_PARSERSCRIPT = 0x17,
        CON_CHANNEL_SCRIPT = 0x18,
        CON_BUILTIN_CHANNEL_COUNT = 0x19,
    };

    typedef void (*Com_Printf_t)(conChannel_t channel, const char *fmt, ...);
    Com_Printf_t Com_Printf = reinterpret_cast<Com_Printf_t>(0x821FFE08);

    typedef void (*Com_PrintError_t)(conChannel_t channel, const char *fmt, ...);
    Com_PrintError_t Com_PrintError = reinterpret_cast<Com_PrintError_t>(0x821FFFA0);

    enum MapType : __int32
    {
        MAPTYPE_NONE = 0x0,
        MAPTYPE_INVALID1 = 0x1,
        MAPTYPE_INVALID2 = 0x2,
        MAPTYPE_2D = 0x3,
        MAPTYPE_3D = 0x4,
        MAPTYPE_CUBE = 0x5,
        MAPTYPE_COUNT = 0x6,
    };

    struct CardMemory
    {
        int platform[1];
    };

    struct GfxImageLoadDef; // Forward declaration

    union GfxTexture
    {
        D3DBaseTexture *basemap;
        D3DTexture *map;
        D3DVolumeTexture *volmap;
        D3DCubeTexture *cubemap;
        GfxImageLoadDef *loadDef;
    };

    struct GfxImageLoadDef
    {
        unsigned __int8 levelCount;
        unsigned __int8 flags;
        __int16 dimensions[3];
        int format;
        GfxTexture texture;
    };

    struct GfxImage
    {
        MapType mapType;
        GfxTexture texture;
        unsigned __int8 semantic;
        CardMemory cardMemory;
        unsigned __int16 width;
        unsigned __int16 height;
        unsigned __int16 depth;
        unsigned __int8 category;
        unsigned __int8 *pixels;
        unsigned int baseSize;
        unsigned __int16 streamSlot;
        bool streaming;
        const char *name;
    };

    struct RawFile
    {
        const char *name;
        int len;
        const char *buffer;
    };

    enum XAssetType : __int32
    {
        ASSET_TYPE_XMODELPIECES = 0x0,
        ASSET_TYPE_PHYSPRESET = 0x1,
        ASSET_TYPE_XANIMPARTS = 0x2,
        ASSET_TYPE_XMODEL = 0x3,
        ASSET_TYPE_MATERIAL = 0x4,
        ASSET_TYPE_PIXELSHADER = 0x5,
        ASSET_TYPE_TECHNIQUE_SET = 0x6,
        ASSET_TYPE_IMAGE = 0x7,
        ASSET_TYPE_SOUND = 0x8,
        ASSET_TYPE_SOUND_CURVE = 0x9,
        ASSET_TYPE_CLIPMAP = 0xA,
        ASSET_TYPE_CLIPMAP_PVS = 0xB,
        ASSET_TYPE_COMWORLD = 0xC,
        ASSET_TYPE_GAMEWORLD_SP = 0xD,
        ASSET_TYPE_GAMEWORLD_MP = 0xE,
        ASSET_TYPE_MAP_ENTS = 0xF,
        ASSET_TYPE_GFXWORLD = 0x10,
        ASSET_TYPE_LIGHT_DEF = 0x11,
        ASSET_TYPE_UI_MAP = 0x12,
        ASSET_TYPE_FONT = 0x13,
        ASSET_TYPE_MENULIST = 0x14,
        ASSET_TYPE_MENU = 0x15,
        ASSET_TYPE_LOCALIZE_ENTRY = 0x16,
        ASSET_TYPE_WEAPON = 0x17,
        ASSET_TYPE_SNDDRIVER_GLOBALS = 0x18,
        ASSET_TYPE_FX = 0x19,
        ASSET_TYPE_IMPACT_FX = 0x1A,
        ASSET_TYPE_AITYPE = 0x1B,
        ASSET_TYPE_MPTYPE = 0x1C,
        ASSET_TYPE_CHARACTER = 0x1D,
        ASSET_TYPE_XMODELALIAS = 0x1E,
        ASSET_TYPE_RAWFILE = 0x1F,
        ASSET_TYPE_STRINGTABLE = 0x20,
        ASSET_TYPE_COUNT = 0x21,
        ASSET_TYPE_STRING = 0x21,
        ASSET_TYPE_ASSETLIST = 0x22,
    };

    union XAssetHeader
    {
        // XModelPieces *xmodelPieces;
        // PhysPreset *physPreset;
        // XAnimParts *parts;
        // XModel *model;
        // Material *material;
        // MaterialPixelShader *pixelShader;
        // MaterialVertexShader *vertexShader;
        // MaterialTechniqueSet *techniqueSet;
        GfxImage *image;
        // snd_alias_list_t *sound;
        // SndCurve *sndCurve;
        // clipMap_t *clipMap;
        // ComWorld *comWorld;
        // GameWorldSp *gameWorldSp;
        // GameWorldMp *gameWorldMp;
        // MapEnts *mapEnts;
        // GfxWorld *gfxWorld;
        // GfxLightDef *lightDef;
        // Font_s *font;
        // MenuList *menuList;
        // menuDef_t *menu;
        // LocalizeEntry *localize;
        // WeaponDef *weapon;
        // SndDriverGlobals *sndDriverGlobals;
        // const FxEffectDef *fx;
        // FxImpactTable *impactFx;
        RawFile *rawfile;
        // StringTable *stringTable;
        void *data;
    };

    typedef int (*DB_GetAllXAssetOfType_t)(XAssetType type, void *assets, int maxCount);
    DB_GetAllXAssetOfType_t DB_GetAllXAssetOfType = reinterpret_cast<DB_GetAllXAssetOfType_t>(0x82265CA8);

    void Cmd_dump_rawfiles_f()
    {
        const int MAX_RAWFILES = 2048;
        XAssetHeader assets[MAX_RAWFILES];
        int count = DB_GetAllXAssetOfType(ASSET_TYPE_RAWFILE, assets, MAX_RAWFILES);
        // Com_Printf(CON_CHANNEL_CONSOLEONLY, "Dumping %d raw files to 'game::\\_iw3xe\\raw\\' %d\n", count);
        for (int i = 0; i < count; i++)
        {
            auto rawfile = assets[i].rawfile;
            std::string asset_name = rawfile->name;
            std::replace(asset_name.begin(), asset_name.end(), '/', '\\'); // Replace forward slashes with backslashes
            filesystem::write_file_to_disk(("game:\\_iw3xe\\dump\\" + asset_name).c_str(), rawfile->buffer, rawfile->len);
        }
        Com_Printf(CON_CHANNEL_CONSOLEONLY, "Dumped %d raw files to 'game::\\_iw3xe\\dump\\'\n", count);
    }

    struct cplane_s;
    struct cStaticModel_s;
    struct dmaterial_t;
    struct cbrushside_t;
    struct cNode_t;
    struct cLeaf_t;
    struct cLeafBrushNode_s;
    struct CollisionBorder;
    struct CollisionPartition;
    struct CollisionAabbTree;
    struct cmodel_t;
    struct cbrush_t;

    struct MapEnts
    {
        const char *name;
        char *entityString;
        int numEntityChars;
    };

    struct clipMap_t
    {
        const char *name;
        int planeCount;
        cplane_s *planes;
        unsigned int numStaticModels;
        cStaticModel_s *staticModelList;
        unsigned int numMaterials;
        dmaterial_t *materials;
        unsigned int numBrushSides;
        cbrushside_t *brushsides;
        unsigned int numBrushEdges;
        unsigned __int8 *brushEdges;
        unsigned int numNodes;
        cNode_t *nodes;
        unsigned int numLeafs;
        cLeaf_t *leafs;
        unsigned int leafbrushNodesCount;
        cLeafBrushNode_s *leafbrushNodes;
        unsigned int numLeafBrushes;
        unsigned __int16 *leafbrushes;
        unsigned int numLeafSurfaces;
        unsigned int *leafsurfaces;
        unsigned int vertCount;
        float (*verts)[3];
        int triCount;
        unsigned __int16 *triIndices;
        unsigned __int8 *triEdgeIsWalkable;
        int borderCount;
        CollisionBorder *borders;
        int partitionCount;
        CollisionPartition *partitions;
        int aabbTreeCount;
        CollisionAabbTree *aabbTrees;
        unsigned int numSubModels;
        cmodel_t *cmodels;
        unsigned __int16 numBrushes;
        cbrush_t *brushes;
        int numClusters;
        int clusterBytes;
        unsigned __int8 *visibility;
        int vised;
        MapEnts *mapEnts;
        // cbrush_t *box_brush;
        // cmodel_t box_model;
        // unsigned __int16 dynEntCount[2];
        // DynEntityDef *dynEntDefList[2];
        // DynEntityPose *dynEntPoseList[2];
        // DynEntityClient *dynEntClientList[2];
        // DynEntityColl *dynEntCollList[2];
        // unsigned int checksum;
    };

    auto cm = reinterpret_cast<clipMap_t *>(0x831C1AC8);

    void Cmd_dump_mapents_f()
    {
        if (!cm->name)
        {
            Com_PrintError(CON_CHANNEL_CONSOLEONLY, "No map loaded\n");
            return;
        }

        // cm.name contains a string like this `maps/mp/mp_backlot.d3dsp`
        std::string asset_name = cm->name;
        asset_name += ".ents";
        std::replace(asset_name.begin(), asset_name.end(), '/', '\\'); // Replace forward slashes with backslashes
        filesystem::write_file_to_disk(("game:\\_iw3xe\\dump\\" + asset_name).c_str(), cm->mapEnts->entityString, cm->mapEnts->numEntityChars);
        Com_Printf(CON_CHANNEL_CONSOLEONLY, "Dumped map ents to 'game:\\_iw3xe\\dump\\%s'\n", cm->mapEnts->numEntityChars, asset_name.c_str());
    }

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

    const uint32_t DDS_MAGIC = MAKEFOURCC('D', 'D', 'S', ' ');
    const uint32_t DDS_HEADER_SIZE = 124;
    const uint32_t DDS_PIXEL_FORMAT_SIZE = 32;
    const uint32_t DDSD_CAPS = 0x1;
    const uint32_t DDSD_HEIGHT = 0x2;
    const uint32_t DDSD_WIDTH = 0x4;
    const uint32_t DDSD_PIXELFORMAT = 0x1000;
    const uint32_t DDSD_LINEARSIZE = 0x80000;
    const uint32_t DDPF_FOURCC = 0x4;
    const uint32_t DDPF_RGB = 0x40;
    const uint32_t DDPF_ALPHAPIXELS = 0x1;
    const uint32_t DDSCAPS_TEXTURE = 0x1000;
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

    void GPUEndianSwapTexture(std::vector<uint8_t> &pixelData, GPUENDIAN endianType)
    {
        switch (endianType)
        {
        case GPUENDIAN_8IN16:
            XGEndianSwapMemory(pixelData.data(), pixelData.data(), XGENDIAN_8IN16, 2, pixelData.size() / 2);
            break;
        case GPUENDIAN_8IN32:
            XGEndianSwapMemory(pixelData.data(), pixelData.data(), XGENDIAN_8IN32, 4, pixelData.size() / 4);
            break;
        case GPUENDIAN_16IN32:
            XGEndianSwapMemory(pixelData.data(), pixelData.data(), XGENDIAN_16IN32, 4, pixelData.size() / 4);
            break;
        }
    }

    void Dump_Image(const GfxImage *image)
    {
        if (!image)
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Null GfxImage!\n");
            return;
        }

        if (image->mapType != MAPTYPE_2D && image->mapType != MAPTYPE_CUBE)
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Unsupported map type %d!\n", image->mapType);
            return;
        }

        UINT BaseSize;
        XGGetTextureLayout(image->texture.basemap, 0, &BaseSize, 0, 0, 0, 0, 0, 0, 0, 0);

        DDSHeader header;
        memset(&header, 0, sizeof(DDSHeader));

        header.magic = _byteswap_ulong(DDS_MAGIC);
        header.size = _byteswap_ulong(DDS_HEADER_SIZE);
        header.flags = _byteswap_ulong(DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE);
        header.height = _byteswap_ulong(image->height);
        header.width = _byteswap_ulong(image->width);
        header.depth = _byteswap_ulong(image->depth);
        header.mipMapCount = _byteswap_ulong(image->texture.basemap->GetLevelCount());

        auto format = image->texture.basemap->Format.DataFormat;
        switch (format)
        {
        case GPUTEXTUREFORMAT_DXT1:
            header.pixelFormat.fourCC = _byteswap_ulong(DXT1_FOURCC);
            header.pitchOrLinearSize = BaseSize;
            break;
        case GPUTEXTUREFORMAT_DXT2_3:
            header.pixelFormat.fourCC = _byteswap_ulong(DXT3_FOURCC);
            header.pitchOrLinearSize = BaseSize;
            break;
        case GPUTEXTUREFORMAT_DXT4_5:
            header.pixelFormat.fourCC = _byteswap_ulong(DXT5_FOURCC);
            header.pitchOrLinearSize = BaseSize;
            break;
        case GPUTEXTUREFORMAT_DXN:
            header.pixelFormat.fourCC = _byteswap_ulong(DXN_FOURCC);
            header.pitchOrLinearSize = BaseSize;
            break;
        case GPUTEXTUREFORMAT_8:
            header.pixelFormat.flags = _byteswap_ulong(DDPF_LUMINANCE);
            header.pixelFormat.rgbBitCount = _byteswap_ulong(8);
            header.pixelFormat.rBitMask = _byteswap_ulong(0x000000FF);
            header.pixelFormat.gBitMask = 0;
            header.pixelFormat.bBitMask = 0;
            header.pixelFormat.aBitMask = 0;
            header.pitchOrLinearSize = BaseSize;
            break;
        case GPUTEXTUREFORMAT_8_8:
            header.pixelFormat.flags = _byteswap_ulong(DDPF_LUMINANCE | DDPF_ALPHAPIXELS);
            header.pixelFormat.rgbBitCount = _byteswap_ulong(16);
            header.pixelFormat.rBitMask = _byteswap_ulong(0x000000FF);
            header.pixelFormat.gBitMask = _byteswap_ulong(0x0000FF00);
            header.pixelFormat.bBitMask = 0;
            header.pixelFormat.aBitMask = 0;
            header.pitchOrLinearSize = BaseSize;
            break;
        case GPUTEXTUREFORMAT_8_8_8_8:
            header.pixelFormat.flags = _byteswap_ulong(DDPF_RGB | DDPF_ALPHAPIXELS);
            header.pixelFormat.rgbBitCount = _byteswap_ulong(32);
            header.pixelFormat.rBitMask = _byteswap_ulong(0x00FF0000);
            header.pixelFormat.gBitMask = _byteswap_ulong(0x0000FF00);
            header.pixelFormat.bBitMask = _byteswap_ulong(0x000000FF);
            header.pixelFormat.aBitMask = _byteswap_ulong(0xFF000000);
            header.pitchOrLinearSize = BaseSize;
            break;
        default:
            Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Unsupported texture format %d!\n", format);
            return;
        }

        // Set texture capabilities
        header.caps = _byteswap_ulong(DDSCAPS_TEXTURE | DDSCAPS_MIPMAP);

        // Handle Cubemaps
        if (image->mapType == MAPTYPE_CUBE)
        {
            header.caps2 = _byteswap_ulong(DDSCAPS2_CUBEMAP |
                                           DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_NEGATIVEX |
                                           DDSCAPS2_CUBEMAP_POSITIVEY | DDSCAPS2_CUBEMAP_NEGATIVEY |
                                           DDSCAPS2_CUBEMAP_POSITIVEZ | DDSCAPS2_CUBEMAP_NEGATIVEZ);
        }

        std::string filename = "game:\\_iw3xe\\dump\\images\\";
        std::string sanitized_name = image->name;

        // Remove invalid characters
        sanitized_name.erase(std::remove_if(sanitized_name.begin(), sanitized_name.end(), [](char c)
                                            { return c == '*'; }),
                             sanitized_name.end());

        filename += sanitized_name + ".dds";

        std::ofstream file(filename, std::ios::binary);
        if (!file)
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Failed to open file: %s\n", filename.c_str());
            return;
        }

        if (image->mapType == MAPTYPE_CUBE)
        {
            file.write(reinterpret_cast<const char *>(&header), sizeof(DDSHeader));

            unsigned int face_size = 0;
            unsigned int rowPitch = 0;
            const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(image->texture.basemap->Format.DataFormat);

            switch (format)
            {
            case GPUTEXTUREFORMAT_DXT1:
                face_size = (image->width / 4) * (image->height / 4) * 8;
                rowPitch = (image->width / 4) * 8; // 8 bytes per 4x4 block
                break;
            case GPUTEXTUREFORMAT_8_8_8_8:
                face_size = image->width * image->height * 4;
                rowPitch = image->width * 4; // 4 bytes per pixel
                break;
            default:
                Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Unsupported cube map format %d!\n", format);
                return;
            }

            // TODO: handle mip levels per face for cubemaps
            for (int i = 0; i < 6; i++)
            {
                unsigned char *face_pixels = image->pixels + (i * face_size); // Offset for each face

                std::vector<uint8_t> swappedFace(face_pixels, face_pixels + face_size);
                GPUEndianSwapTexture(swappedFace, static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

                // Create buffer for linear texture data
                std::vector<uint8_t> linearFace(face_size);

                // Convert tiled texture to linear layout using XGUntileTextureLevel
                XGUntileTextureLevel(
                    image->width,               // Width
                    image->height,              // Height
                    0,                          // Level (base level)
                    static_cast<DWORD>(format), // GpuFormat
                    0,                          // Flags (no special flags)
                    linearFace.data(),          // pDestination (linear output)
                    rowPitch,                   // RowPitch
                    nullptr,                    // pPoint (no offset)
                    swappedFace.data(),         // pSource (tiled input)
                    nullptr                     // pRect (entire texture)
                );

                file.write(reinterpret_cast<const char *>(linearFace.data()), linearFace.size());
            }

            file.close();
        }
        else if (image->mapType == MAPTYPE_2D)
        {
            // TODO: write mip levels
            file.write(reinterpret_cast<const char *>(&header), sizeof(DDSHeader));

            std::vector<uint8_t> pixelData(image->pixels, image->pixels + image->baseSize);

            GPUEndianSwapTexture(pixelData, static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

            // Create a linear data buffer to hold the untiled texture
            std::vector<uint8_t> linearData(image->baseSize);

            // Calculate row pitch based on format
            UINT rowPitch;
            auto format = image->texture.basemap->Format.DataFormat;

            switch (format)
            {
            case GPUTEXTUREFORMAT_DXT1:
            case GPUTEXTUREFORMAT_DXT2_3:
            case GPUTEXTUREFORMAT_DXT4_5:
            case GPUTEXTUREFORMAT_DXN:
                // Block compressed formats use 4x4 blocks
                rowPitch = ((image->width + 3) / 4) * (format == GPUTEXTUREFORMAT_DXT1 ? 8 : 16);
                break;
            case GPUTEXTUREFORMAT_8:
                rowPitch = image->width;
                break;
            case GPUTEXTUREFORMAT_8_8:
                rowPitch = image->width * 2;
                break;
            case GPUTEXTUREFORMAT_8_8_8_8:
                rowPitch = image->width * 4;
                break;
            default:
                Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Unsupported texture format %d!\n", format);
                return;
            }

            xbox::DbgPrint("Image_Dump: rowPitch=%d\n", rowPitch);

            // Call XGUntileTextureLevel to convert the tiled texture to linear format
            XGUntileTextureLevel(
                image->width,               // Width
                image->height,              // Height
                0,                          // Level (base level 0)
                static_cast<DWORD>(format), // GpuFormat
                XGTILE_NONPACKED,           // Flags (no special flags)
                linearData.data(),          // pDestination
                rowPitch,                   // RowPitch (calculated based on format)
                nullptr,                    // pPoint (no offset)
                pixelData.data(),           // pSource
                nullptr                     // pRect (entire texture)
            );

            file.write(reinterpret_cast<const char *>(linearData.data()), linearData.size());

            file.close();
        }
        else
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Image_Dump: Unsupported map type %d!\n", image->mapType);
            return;
        }
    }

    void Cmd_dump_images_f()
    {
        CreateDirectoryA("game:\\_iw3xe", nullptr);
        CreateDirectoryA("game:\\_iw3xe\\dump", nullptr);
        CreateDirectoryA("game:\\_iw3xe\\dump\\images", nullptr);

        const int MAX = 2048;
        XAssetHeader assets[MAX];
        int count = DB_GetAllXAssetOfType(ASSET_TYPE_IMAGE, assets, MAX);

        for (int i = 0; i < count; i++)
        {
            Dump_Image(assets[i].image);
        }
        Com_Printf(CON_CHANNEL_CONSOLEONLY, "Dumped %d rimages to 'game:\\_iw3xe\\dump\\images'\n", count);
    }

    // Detour CG_InitConsoleCommands_Detour;

    // void CG_InitConsoleCommands_Hook()
    // {
    //     xbox::DbgPrint("CG_InitConsoleCommands_Hook\n");
    //     CG_InitConsoleCommands_Detour.GetOriginal<decltype(CG_InitConsoleCommands)>()();
    //     // Add your custom command here

    //     cmd_function_s *rawfilesdump_VAR = new cmd_function_s;
    //     Cmd_AddCommandInternal("rawfiledump", Cmd_rawfilesdump, rawfilesdump_VAR);
    // }

    void init_cg_consolecmds()
    {
        // xbox::DbgPrint("Initializing CG_InitConsoleCommands detour...\n");
        // CG_InitConsoleCommands_Detour = Detour(CG_InitConsoleCommands, CG_InitConsoleCommands_Hook);
        // CG_InitConsoleCommands_Detour.Install();

        cmd_function_s *dump_images_VAR = new cmd_function_s;
        Cmd_AddCommandInternal("dump_images", Cmd_dump_images_f, dump_images_VAR);

        cmd_function_s *dump_mapents_VAR = new cmd_function_s;
        Cmd_AddCommandInternal("dump_mapents", Cmd_dump_mapents_f, dump_mapents_VAR);

        cmd_function_s *dump_rawfiles_VAR = new cmd_function_s;
        Cmd_AddCommandInternal("dump_rawfiles", Cmd_dump_rawfiles_f, dump_rawfiles_VAR);
    }
}