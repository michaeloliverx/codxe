#pragma warning(push)
#pragma warning(disable : 4480)

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
        // GfxImage *image;
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

        cmd_function_s *dump_mapents_VAR = new cmd_function_s;
        Cmd_AddCommandInternal("dump_mapents", Cmd_dump_mapents_f, dump_mapents_VAR);

        cmd_function_s *dump_rawfiles_VAR = new cmd_function_s;
        Cmd_AddCommandInternal("dump_rawfiles", Cmd_dump_rawfiles_f, dump_rawfiles_VAR);
    }
}