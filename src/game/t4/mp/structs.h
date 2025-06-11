#pragma once

#pragma warning(disable : 4480) // nonstandard extension used: specifying underlying type for enum

namespace t4
{
    namespace mp
    {
        enum XAssetType : __int32
        {
            ASSET_TYPE_IMAGE = 0x9,
            ASSET_TYPE_FONT = 0x15,
        };

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

        union GfxTexture
        {
            D3DBaseTexture *basemap;
            D3DTexture *map;
            D3DVolumeTexture *volmap;
            D3DCubeTexture *cubemap;
            struct GfxImageLoadDef *loadDef; // forward declaration
        };

        struct GfxImageLoadDef
        {
            unsigned __int8 levelCount;
            unsigned __int8 flags;
            __int16 dimensions[3];
            int format;
            GfxTexture texture;
        };

        struct CardMemory
        {
            int platform[1];
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
            bool delayLoadPixels;
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

        struct Material;

        struct Glyph
        {
            unsigned __int16 letter;
            char x0;
            char y0;
            unsigned __int8 dx;
            unsigned __int8 pixelWidth;
            unsigned __int8 pixelHeight;
            float s0;
            float t0;
            float s1;
            float t1;
        };
        static_assert(sizeof(Glyph) == 0x18, "");

        struct Font_s
        {
            const char *fontName;
            int pixelHeight;
            int glyphCount;
            Material *material;
            Material *glowMaterial;
            Glyph *glyphs;
        };
        static_assert(sizeof(Font_s) == 0x18, "");

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
            // LoadedSound *loadSnd;
            // clipMap_t *clipMap;
            // ComWorld *comWorld;
            // GameWorldSp *gameWorldSp;
            // GameWorldMp *gameWorldMp;
            // MapEnts *mapEnts;
            // GfxWorld *gfxWorld;
            // GfxLightDef *lightDef;
            Font_s *font;
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

        struct cplane_s
        {
            float normal[3];
            float dist;
            unsigned __int8 type;
            unsigned __int8 signbits;
            unsigned __int8 pad[2];
        };

        struct __declspec(align(2)) cbrushside_t
        {
            cplane_s *plane;
            unsigned int materialNum;
            __int16 firstAdjacentSideOffset;
            unsigned __int8 edgeCount;
        };

#pragma warning(disable : 4324)
        struct __declspec(align(16)) cbrush_t
        {
            float mins[3];
            int contents;
            float maxs[3];
            unsigned int numsides;
            cbrushside_t *sides;
            __int16 axialMaterialNum[2][3];
            unsigned __int8 *baseAdjacentSide;
            __int16 firstAdjacentSideOffsets[2][3];
            unsigned __int8 edgeCount[2][3];
        };
#pragma warning(default : 4324)

        struct clipMap_t
        {
            const char *name;
            int isInUse;
            char _pad[0x94];
            unsigned __int16 numBrushes;
            cbrush_t *brushes;
        };
        static_assert(offsetof(clipMap_t, numBrushes) == 156, "");

        struct gclient_s
        {
            char pad_0000[44];         // Padding to reach offset 44
            float velocity[3];         // Offset 44
            char pad_0056[15076];      // Padding to reach offset 15132
            int noclip;                // Offset 15132
            int ufo;                   // Offset 15136
            int bFrozen;               //
            int lastCmdTime;           //
            int buttons;               // Offset 15148
            int oldbuttons;            //
            int latched_buttons;       //
            int buttonsSinceLastFrame; // Offset 15160
        };
        static_assert(offsetof(gclient_s, noclip) == 15132, "");
        static_assert(offsetof(gclient_s, ufo) == 15136, "");
        static_assert(offsetof(gclient_s, buttons) == 15148, "");
        static_assert(offsetof(gclient_s, buttonsSinceLastFrame) == 15160, "");

        struct gentity_s
        {
            char pad_0000[388];
            gclient_s *client; // Offset 388
            char pad_0188[44]; //
            int flags;         // Offset 436
            char pad_01BC[816 - 440];
        };
        static_assert(sizeof(gentity_s) == 816, "");

        enum fieldtype_t : __int32
        {
            F_INT = 0x0,
            F_SHORT = 0x1,
            F_BYTE = 0x2,
            F_FLOAT = 0x3,
            F_LSTRING = 0x4,
            F_STRING = 0x5,
            F_VECTOR = 0x6,
            F_ENTITY = 0x7,
            F_ENTHANDLE = 0x8,
            F_ACTOR = 0x9,
            F_SENTIENT = 0xA,
            F_SENTIENTHANDLE = 0xB,
            F_CLIENT = 0xC,
            F_PATHNODE = 0xD,
            F_VECTORHACK = 0xE,
            F_MODEL = 0xF,
            F_OBJECT = 0x10,
            F_ACTORGROUP = 0x11,
            F_BITFLAG = 0x12,
        };

        struct client_fields_s;

        typedef void (*ClientFieldSetter)(gclient_s *pSelf, client_fields_s *pField);
        typedef void (*ClientFieldGetter)(gclient_s *pSelf, client_fields_s *pField);

        struct client_fields_s
        {
            char *name;
            int ofs;
            fieldtype_t type;
            unsigned int whichbits;
            ClientFieldSetter setter;
            ClientFieldGetter getter;
        };
        static_assert(sizeof(client_fields_s) == 24, "");

        struct scr_entref_t
        {
            unsigned __int16 entnum;
            unsigned __int16 classnum;
        };

        enum button_mask
        {
            KEY_FIRE = 0x1,
            KEY_SPRINT = 0x2,
            KEY_MELEE = 0x4,
            KEY_USE = 0x8,
            KEY_RELOAD = 0x10,
            KEY_USERELOAD = 0x20,
            KEY_LEANLEFT = 0x40,
            KEY_LEANRIGHT = 0x80,
            KEY_PRONE = 0x100,
            KEY_CROUCH = 0x200,
            KEY_GOSTAND = 0x400,
            KEY_ADSMODE = 0x800,
            KEY_TEMP = 0x1000,
            KEY_HOLDBREATH = 0x2000,
            KEY_FRAG = 0x4000,
            KEY_SMOKE = 0x8000,
            KEY_SELECTING_LOCATION = 0x10000,
            KEY_CANCEL_LOCATION = 0x20000,
            KEY_NIGHTVISION = 0x40000,
            KEY_ADS = 0x80000,
            KEY_REVERSE = 0x100000,
            KEY_HANDBRAKE = 0x200000,
            KEY_THROW = 0x400000,
            KEY_INMENU = 0x800000,
        };

        enum svscmd_type
        {
            SV_CMD_CAN_IGNORE = 0x0,
            SV_CMD_RELIABLE = 0x1,
        };

        enum scriptInstance_t : __int32
        {
            SCRIPTINSTANCE_SERVER = 0x0,
            SCRIPTINSTANCE_CLIENT = 0x1,
            SCRIPT_INSTANCE_MAX = 0x2,
        };
    }
}
