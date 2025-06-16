#pragma once

#pragma warning(disable : 4480) // nonstandard extension used: specifying underlying type for enum

// usercmd_t->button bits
#define KEY_MASK_FIRE 1
#define KEY_MASK_SPRINT 2
#define KEY_MASK_MELEE 4
#define KEY_MASK_RELOAD 16
#define KEY_MASK_LEANLEFT 64
#define KEY_MASK_LEANRIGHT 128
#define KEY_MASK_PRONE 256
#define KEY_MASK_CROUCH 512
#define KEY_MASK_JUMP 1024
#define KEY_MASK_ADS_MODE 2048
#define KEY_MASK_TEMP_ACTION 4096
#define KEY_MASK_HOLDBREATH 8192
#define KEY_MASK_FRAG 16384
#define KEY_MASK_SMOKE 32768
#define KEY_MASK_NIGHTVISION 262144
#define KEY_MASK_ADS 524288
#define KEY_MASK_USE 8
#define KEY_MASK_USERELOAD 0x20
#define BUTTON_ATTACK KEY_MASK_FIRE

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

        struct playerState_s
        {
            char pad[44];
            float velocity[3];
            char pad_0056[192];
            int clientNum;
            char pad_end[14880];
        };

        static_assert(offsetof(playerState_s, velocity) == 44, "");
        static_assert(offsetof(playerState_s, clientNum) == 248, "");

        struct gclient_s
        {
            playerState_s ps;
            int noclip;                // Offset 15132
            int ufo;                   // Offset 15136
            int bFrozen;               //
            int lastCmdTime;           //
            int buttons;               // Offset 15148
            int oldbuttons;            //
            int latched_buttons;       //
            int buttonsSinceLastFrame; // Offset 15160
            char pad[15468 - 15164];   // Padding to reach end of struct
        };
        static_assert(offsetof(gclient_s, noclip) == 15132, "");
        static_assert(offsetof(gclient_s, ufo) == 15136, "");
        static_assert(offsetof(gclient_s, buttons) == 15148, "");
        static_assert(offsetof(gclient_s, buttonsSinceLastFrame) == 15160, "");
        static_assert(sizeof(gclient_s) == 15468, "");

        enum gentity_flags_t : __int32
        {
            FL_GODMODE = 1 << 0,
            FL_DEMI_GODMODE = 1 << 1,
        };

        struct gentity_s
        {
            char pad_0000[388];
            gclient_s *client;
            char pad_0188[44];
            int flags;
            char pad_01BC[816 - 440];
        };
        static_assert(offsetof(gentity_s, client) == 388, "");
        static_assert(offsetof(gentity_s, flags) == 436, "");
        static_assert(sizeof(gentity_s) == 816, "");

        struct level_locals_t
        {
            gclient_s *clients;
        };

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

        typedef void (*ClientFieldSetter)(gclient_s *pSelf, const client_fields_s *pField);
        typedef void (*ClientFieldGetter)(gclient_s *pSelf, const client_fields_s *pField);

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

        typedef void (*BuiltinFunction)();

        struct BuiltinFunctionDef
        {
            const char *actionString;   // OFS: 0x0 SIZE: 0x4
            BuiltinFunction actionFunc; // OFS: 0x4 SIZE: 0x4
            int type;                   // OFS: 0x8 SIZE: 0x4
        };
        static_assert(sizeof(BuiltinFunctionDef) == 0xC, "");
        static_assert(offsetof(BuiltinFunctionDef, actionString) == 0x0, "");
        static_assert(offsetof(BuiltinFunctionDef, actionFunc) == 0x4, "");
        static_assert(offsetof(BuiltinFunctionDef, type) == 0x8, "");

        typedef void (*BuiltinPlayerMethod)(scr_entref_t entref);

        struct BuiltinMethodDef
        {
            const char *actionString;
            BuiltinPlayerMethod actionFunc;
            int type;
        };
        static_assert(sizeof(BuiltinMethodDef) == 0xC, "");
        static_assert(offsetof(BuiltinMethodDef, actionString) == 0x0, "");
        static_assert(offsetof(BuiltinMethodDef, actionFunc) == 0x4, "");
        static_assert(offsetof(BuiltinMethodDef, type) == 0x8, "");

        enum DvarFlags : unsigned __int16
        {
            DVAR_FLAG_NONE = 0x0,
            DVAR_CODINFO = 0x100, // On change, this is sent to all clients (if you are host)
        };

        enum dvarType_t : __int8
        {
            DVAR_TYPE_BOOL = 0x0,
            DVAR_TYPE_FLOAT = 0x1,
            DVAR_TYPE_FLOAT_2 = 0x2,
            DVAR_TYPE_FLOAT_3 = 0x3,
            DVAR_TYPE_FLOAT_4 = 0x4,
            DVAR_TYPE_INT = 0x5,
            DVAR_TYPE_ENUM = 0x6,
            DVAR_TYPE_STRING = 0x7,
            DVAR_TYPE_COLOR = 0x8,
            DVAR_TYPE_COUNT = 0x9,
        };

        union DvarValue
        {
            bool enabled;             // OFS: 0x0 SIZE: 0x1
            int integer;              // OFS: 0x1 SIZE: 0x4
            unsigned int unsignedInt; // OFS: 0x2 SIZE: 0x4
            float value;              // OFS: 0x3 SIZE: 0x4
            float vector[4];          // OFS: 0x4 SIZE: 0x10
            const char *string;       // OFS: 0x5 SIZE: 0x4
            char color[4];            // OFS: 0x6 SIZE: 0x4
        };
        static_assert(sizeof(DvarValue) == 0x10, "");

        struct DvarLimits_enum
        {
            int stringCount;      // OFS: 0x0 SIZE: 0x4
            const char **strings; // OFS: 0x4 SIZE: 0x4
        };
        static_assert(sizeof(DvarLimits_enum) == 0x8, "");
        static_assert(offsetof(DvarLimits_enum, stringCount) == 0x0, "");
        static_assert(offsetof(DvarLimits_enum, strings) == 0x4, "");

        struct DvarLimits_minmax
        {
            int min; // OFS: 0x0 SIZE: 0x4
            int max; // OFS: 0x4 SIZE: 0x4
        };
        static_assert(sizeof(DvarLimits_minmax) == 0x8, "");
        static_assert(offsetof(DvarLimits_minmax, min) == 0x0, "");
        static_assert(offsetof(DvarLimits_minmax, max) == 0x4, "");

        struct DvarLimits_float_minmax
        {
            float min; // OFS: 0x0 SIZE: 0x4
            float max; // OFS: 0x4 SIZE: 0x4
        };
        static_assert(sizeof(DvarLimits_float_minmax) == 0x8, "");
        static_assert(offsetof(DvarLimits_float_minmax, min) == 0x0, "");
        static_assert(offsetof(DvarLimits_float_minmax, max) == 0x4, "");

        union DvarLimits
        {
            DvarLimits_enum enumeration;    // OFS: 0x0 SIZE: 0x8
            DvarLimits_minmax integer;      // OFS: 0x1 SIZE: 0x8
            DvarLimits_float_minmax value;  // OFS: 0x2 SIZE: 0x8
            DvarLimits_float_minmax vector; // OFS: 0x3 SIZE: 0x8
        };
        static_assert(sizeof(DvarLimits) == 0x8, "");

        struct dvar_s
        {
            const char *name;        // OFS: 0x0 SIZE: 0x4
            const char *description; // OFS: 0x4 SIZE: 0x4
            DvarFlags flags;         // OFS: 0x8 SIZE: 0x2
            dvarType_t type;         // OFS: 0xA SIZE: 0x1
            char modified;           // OFS: 0xB SIZE: 0x1
            char saveRestorable;     // OFS: 0xC SIZE: 0x1
            DvarValue current;       // OFS: 0x10 SIZE: 0x10
            DvarValue latched;       // OFS: 0x20 SIZE: 0x10
            DvarValue reset;         // OFS: 0x30 SIZE: 0x10
            DvarValue saved;         // OFS: 0x40 SIZE: 0x10
            DvarLimits domain;       // OFS: 0x50 SIZE: 0x8
            dvar_s *hashNext;        // OFS: 0x58 SIZE: 0x4
        };
        static_assert(offsetof(dvar_s, name) == 0x0, "");
        static_assert(offsetof(dvar_s, description) == 0x4, "");
        static_assert(offsetof(dvar_s, flags) == 0x8, "");
        static_assert(offsetof(dvar_s, type) == 0xA, "");
        static_assert(offsetof(dvar_s, modified) == 0xB, "");
        static_assert(offsetof(dvar_s, saveRestorable) == 0xC, "");
        static_assert(offsetof(dvar_s, current) == 0x10, "");
        static_assert(offsetof(dvar_s, latched) == 0x20, "");
        static_assert(offsetof(dvar_s, reset) == 0x30, "");
        static_assert(offsetof(dvar_s, saved) == 0x40, "");
        static_assert(offsetof(dvar_s, domain) == 0x50, "");
        static_assert(offsetof(dvar_s, hashNext) == 0x58, "");
        static_assert(sizeof(dvar_s) == 0x5C, "");

        struct viewState_t;
        struct weaponState_t;
    }
}
