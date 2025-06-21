#pragma once

#include <cstddef>

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

        struct MapEnts
        {
            const char *name;   // OFS: 0x0 SIZE: 0x4
            char *entityString; // OFS: 0x4 SIZE: 0x4
            int numEntityChars; // OFS: 0x8 SIZE: 0x4
        };
        static_assert(sizeof(MapEnts) == 12, "");
        static_assert(offsetof(MapEnts, name) == 0x0, "");
        static_assert(offsetof(MapEnts, entityString) == 0x4, "");
        static_assert(offsetof(MapEnts, numEntityChars) == 0x8, "");

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
            MapEnts *mapEnts;
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

        struct __declspec(align(4)) cLeaf_t
        {
            unsigned __int16 firstCollAabbIndex;
            unsigned __int16 collAabbCount;
            int brushContents;
            int terrainContents;
            float mins[3];
            float maxs[3];
            int leafBrushNode;
            __int16 cluster;
        };
        static_assert(sizeof(cLeaf_t) == 44, "");

        struct cmodel_t
        {
            float mins[3];
            float maxs[3];
            float radius;
            cLeaf_t leaf;
        };
        static_assert(sizeof(cmodel_t) == 72, "");
        static_assert(offsetof(cmodel_t, mins) == 0x0, "");
        static_assert(offsetof(cmodel_t, maxs) == 12, "");
        static_assert(offsetof(cmodel_t, radius) == 24, "");
        static_assert(offsetof(cmodel_t, leaf) == 28, "");

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
            const char *name;            // OFS: 0 SIZE: 0x4
            int isInUse;                 // OFS: 4 SIZE: 0x4
            char _pad[140];              //
            unsigned int numSubModels;   // OFS: 148 SIZE: 0x4
            cmodel_t *cmodels;           // OFS: 152 SIZE: 0x4
            unsigned __int16 numBrushes; // OFS: 156 SIZE: 0x2
            cbrush_t *brushes;           // OFS: 160 SIZE: 0x4
            char _pad2[16];              //
            MapEnts *mapEnts;            // OFS: 180 SIZE: 0xC
        };
        static_assert(offsetof(clipMap_t, name) == 0, "");
        static_assert(offsetof(clipMap_t, isInUse) == 4, "");
        static_assert(offsetof(clipMap_t, numSubModels) == 148, "");
        static_assert(offsetof(clipMap_t, cmodels) == 152, "");
        static_assert(offsetof(clipMap_t, numBrushes) == 156, "");
        static_assert(offsetof(clipMap_t, brushes) == 160, "");
        static_assert(offsetof(clipMap_t, mapEnts) == 180, "");

        struct playerState_s
        {
            char pad0[12];        //
            int pm_flags;         // OFS: 12 SIZE: 0x4
            char pad[16];         //
            float origin[3];      // OFS: 32 SIZE: 12
            float velocity[3];    // OFS: 44 SIZE: 12
            char pad_0056[148];   //
            int eFlags;           // OFS: 204 SIZE: 0x4
            char pad_end[40];     //
            int clientNum;        // OFS: 248 SIZE:0x4
            char pad_end2[14880]; //
        };
        static_assert(sizeof(playerState_s) == 15132, "");
        static_assert(offsetof(playerState_s, pm_flags) == 12, "");
        static_assert(offsetof(playerState_s, origin) == 32, "");
        static_assert(offsetof(playerState_s, velocity) == 44, "");
        static_assert(offsetof(playerState_s, eFlags) == 204, "");
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

        enum trType_t : __int32
        {
            TR_STATIONARY = 0x0,
            TR_INTERPOLATE = 0x1,
            TR_LINEAR = 0x2,
            TR_LINEAR_STOP = 0x3,
            TR_SINE = 0x4,
            TR_GRAVITY = 0x5,
            TR_ACCELERATE = 0x6,
            TR_DECELERATE = 0x7,
            TR_PHYSICS = 0x8,
            TR_XDOLL = 0x9,
            TR_FIRST_RAGDOLL = 0xA,
            TR_RAGDOLL = 0xA,
            TR_RAGDOLL_GRAVITY = 0xB,
            TR_RAGDOLL_INTERPOLATE = 0xC,
            TR_LAST_RAGDOLL = 0xC,
            TR_COUNT = 0xD,
        };

        struct trajectory_t
        {
            trType_t trType;
            int trTime;
            int trDuration;
            float trBase[3];
            float trDelta[3];
        };

        struct LerpEntityStateTurret
        {
            float gunAngles[3];
            unsigned __int8 overheating;
            float heatVal;
        };
        static_assert(sizeof(LerpEntityStateTurret) == 0x14, "");
        static_assert(offsetof(LerpEntityStateTurret, gunAngles) == 0x0, "");
        static_assert(offsetof(LerpEntityStateTurret, overheating) == 0xC, "");
        static_assert(offsetof(LerpEntityStateTurret, heatVal) == 0x10, "");

        struct LerpEntityStateLoopFx
        {
            float cullDist;
            int period;
        };
        static_assert(sizeof(LerpEntityStateLoopFx) == 0x8, "");
        static_assert(offsetof(LerpEntityStateLoopFx, cullDist) == 0x0, "");
        static_assert(offsetof(LerpEntityStateLoopFx, period) == 0x4, "");

        union LerpEntityStateActor_unnamed_type_index
        {
            int actorNum;
            int corpseNum;
        };

        struct LerpEntityStateActor_unnamed_type_proneInfo
        {
            __int16 fBodyPitch;
        };

        struct __declspec(align(4)) LerpEntityStateActor
        {
            LerpEntityStateActor_unnamed_type_index index;
            int species;
            LerpEntityStateActor_unnamed_type_proneInfo proneInfo;
        };

        struct LerpEntityStatePrimaryLight
        {
            char colorAndExp[4];
            float intensity;
            float radius;
            float cosHalfFovOuter;
            float cosHalfFovInner;
        };

        struct __declspec(align(4)) LerpEntityStatePlayer
        {
            float leanf;
            int movementDir;
            char vehicleType;
            char vehicleAnimBoneIndex;
            char vehicleSeat;
        };

        struct LerpEntityStateVehicleGunnerAngles
        {
            __int16 pitch;
            __int16 yaw;
        };

        union LerpEntityStateVehicleThrottle_u
        {
            __int16 throttle;
            __int16 bodyPitch;
        };

        struct __declspec(align(2)) LerpEntityStateVehicle
        {
            float steerYaw;
            float bodyRoll;
            float bodyPitch;
            LerpEntityStateVehicleGunnerAngles gunnerAngles[4];
            LerpEntityStateVehicleThrottle_u ___u3;
            __int16 gunPitch;
            __int16 gunYaw;
            char drawOnCompass;
            int teamAndOwnerIndex;
        };

        struct LerpEntityStateMissile
        {
            int launchTime;
            int parentClientNum;
            int fuseTime;
        };

        struct LerpEntityStateScriptMover
        {
            char attachTagIndex[4];
            int attachedTagIndex;
            __int16 attachModelIndex[4];
            __int16 animScriptedAnim;
            int animScriptedAnimTime;
            __int16 attachedEntNum;
            __int16 exploderIndex;
        };

        struct LerpEntityStateSoundBlend
        {
            float lerp;
            float volumeScale;
        };

        struct LerpEntityStateAnonymous
        {
            int data[15];
        };

        union LerpEntityStateTypeUnion
        {
            LerpEntityStateTurret turret;
            LerpEntityStateLoopFx loopFx;
            LerpEntityStateActor actor;
            LerpEntityStatePrimaryLight primaryLight;
            LerpEntityStatePlayer player;
            LerpEntityStateVehicle vehicle;
            LerpEntityStateMissile missile;
            LerpEntityStateScriptMover scriptMover;
            LerpEntityStateSoundBlend soundBlend;
            LerpEntityStateAnonymous anonymous;
        };

        struct __declspec(align(4)) LerpEntityState
        {
            int eFlags;
            trajectory_t pos;
            trajectory_t apos;
            LerpEntityStateTypeUnion u;
            int usecount;
        };
        static_assert(sizeof(LerpEntityState) == 140, "");

        enum entityType_t : __int32
        {
            ET_GENERAL = 0x0,
            ET_PLAYER = 0x1,
            ET_PLAYER_CORPSE = 0x2,
            ET_ITEM = 0x3,
            ET_MISSILE = 0x4,
            ET_INVISIBLE = 0x5,
            ET_SCRIPTMOVER = 0x6,
            ET_SOUND_BLEND = 0x7,
            ET_FX = 0x8,
            ET_LOOP_FX = 0x9,
            ET_PRIMARY_LIGHT = 0xA,
            ET_MG42 = 0xB,
            ET_PLANE = 0xC,
            ET_VEHICLE = 0xD,
            ET_VEHICLE_COLLMAP = 0xE,
            ET_VEHICLE_CORPSE = 0xF,
            ET_ACTOR = 0x10,
            ET_ACTOR_SPAWNER = 0x11,
            ET_ACTOR_CORPSE = 0x12,
            ET_EVENTS = 0x13,
        };

        struct LoopSound
        {
            unsigned __int16 soundAlias;
            __int16 fadeTime;
        };

        union entityState_index
        {
            int brushmodel;
            int item;
            int xmodel;
            int primaryLight;
        };

        struct entityState_s
        {
            int number;               // OFS: 0x0 SIZE: 0x4
            entityType_t eType;       // OFS: 4 SIZE: 0x4
            LerpEntityState lerp;     // OFS: 8 SIZE: 140
            int time2;                // OFS: 148 SIZE: 0x4
            int otherEntityNum;       // OFS: 152 SIZE: 0x4
            int attackerEntityNum;    // OFS: 156 SIZE: 0x4
            int groundEntityNum;      // OFS: 160 SIZE: 0x4
            LoopSound loopSound;      // OFS: 164 SIZE: 0x4
            int surfType;             // OFS: 168 SIZE: 0x4
            entityState_index index;  // OFS: 172 SIZE: 0x4
            int clientNum;            // OFS: 176 SIZE: 0x4
            int iHeadIcon;            // OFS: 180 SIZE: 0x4
            int iHeadIconTeam;        // OFS: 184 SIZE: 0x4
            int solid;                // OFS: 188 SIZE: 0x4
            int eventParm;            // OFS: 192 SIZE: 0x4
            int eventSequence;        // OFS: 196 SIZE: 0x4
            int events[4];            // OFS: 200 SIZE: 0x10
            int eventParms[4];        // OFS: 216 SIZE: 0x10
            int weapon;               // OFS: 232 SIZE: 0x4
            int weaponModel;          // OFS: 236 SIZE: 0x4
            int targetName;           // OFS: 240 SIZE: 0x4
            char pad00[24];           //
            unsigned int partBits[4]; // OFS: 268 SIZE: 0x10
        };
        static_assert(sizeof(entityState_s) == 284, "");
        static_assert(offsetof(entityState_s, number) == 0, "");
        static_assert(offsetof(entityState_s, eType) == 4, "");
        static_assert(offsetof(entityState_s, lerp) == 8, "");
        static_assert(offsetof(entityState_s, time2) == 148, "");
        static_assert(offsetof(entityState_s, otherEntityNum) == 152, "");
        static_assert(offsetof(entityState_s, attackerEntityNum) == 156, "");
        static_assert(offsetof(entityState_s, groundEntityNum) == 160, "");
        static_assert(offsetof(entityState_s, loopSound) == 164, "");
        static_assert(offsetof(entityState_s, surfType) == 168, "");
        static_assert(offsetof(entityState_s, index) == 172, "");
        static_assert(offsetof(entityState_s, clientNum) == 176, "");
        static_assert(offsetof(entityState_s, iHeadIcon) == 180, "");
        static_assert(offsetof(entityState_s, iHeadIconTeam) == 184, "");
        static_assert(offsetof(entityState_s, solid) == 188, "");
        static_assert(offsetof(entityState_s, eventParm) == 192, "");
        static_assert(offsetof(entityState_s, eventSequence) == 196, "");
        static_assert(offsetof(entityState_s, events) == 200, "");
        static_assert(offsetof(entityState_s, eventParms) == 216, "");
        static_assert(offsetof(entityState_s, weapon) == 232, "");
        static_assert(offsetof(entityState_s, weaponModel) == 236, "");
        static_assert(offsetof(entityState_s, targetName) == 240, "");
        static_assert(offsetof(entityState_s, partBits) == 268, "");

        struct entityShared_t
        {
            unsigned __int8 linked;   // OFS: 0 SIZE: 0x1
            unsigned __int8 bmodel;   // OFS: 2 SIZE: 0x1
            unsigned __int16 svFlags; // OFS: 3 SIZE: 0x1
            int clientMask[2];        // OFS: 8 SIZE: 0x8
            float absmin[3];          //
            float absmax[3];          // OFS: 24 SIZE: 0xC
            char pad0[8];             //
            int contents;             // OFS: 44 SIZE: 0x4
            char pad1[24];            //
            float currentOrigin[3];   // OFS: 72 SIZE: 0xC
            float currentAngles[3];   // OFS: 84 SIZE: 0xC
            char pad2[8];             //
        };
        static_assert(sizeof(entityShared_t) == 104, "");
        static_assert(offsetof(entityShared_t, linked) == 0, "");
        static_assert(offsetof(entityShared_t, bmodel) == 1, "");
        static_assert(offsetof(entityShared_t, svFlags) == 2, "");
        static_assert(offsetof(entityShared_t, clientMask) == 4, "");
        static_assert(offsetof(entityShared_t, absmin) == 12, "");
        static_assert(offsetof(entityShared_t, absmax) == 24, "");
        static_assert(offsetof(entityShared_t, contents) == 44, "");
        static_assert(offsetof(entityShared_t, currentOrigin) == 72, "");
        static_assert(offsetof(entityShared_t, currentAngles) == 84, "");

        struct gentity_s
        {
            entityState_s s;                    // OFS: 0 SIZE: 284
            entityShared_t r;                   // OFS: 284 SIZE: 104
            gclient_s *client;                  // OFS: 388 SIZE: 0x4
            char pad0[20];                      //
            unsigned __int16 model;             // OFS: 412 SIZE: 0x2
            char pad1[6];                       //
            unsigned __int16 classname;         // OFS: 420 SIZE: 0x2
            unsigned __int16 target;            // OFS: 422 SIZE: 0x2
            unsigned __int16 targetname;        // OFS: 424 SIZE: 0x2
            unsigned __int16 script_noteworthy; // OFS: 426 SIZE: 0x2
            char pad2[2];                       //
            int spawnflags;                     // OFS: 432 SIZE: 0x4
            int flags;                          // OFS: 436 SIZE: 0x4
            char pad[28];                       //
            int health;                         // OFS: 468 SIZE: 0x4
            int maxHealth;                      // OFS: 472 SIZE: 0x4
            int dmg;                            // OFS: 476 SIZE: 0x4
            char pad3[332];                     //
            int birthtime;                      // OFS: 812 SIZE: 0x4
        };
        static_assert(sizeof(gentity_s) == 816, "");

        static_assert(offsetof(gentity_s, s) == 0, "");
        static_assert(offsetof(gentity_s, s.number) == 0, "");
        static_assert(offsetof(gentity_s, s.eType) == 4, "");
        static_assert(offsetof(gentity_s, s.lerp) == 8, "");
        static_assert(offsetof(gentity_s, s.time2) == 148, "");
        static_assert(offsetof(gentity_s, s.otherEntityNum) == 152, "");
        static_assert(offsetof(gentity_s, s.attackerEntityNum) == 156, "");
        static_assert(offsetof(gentity_s, s.groundEntityNum) == 160, "");
        static_assert(offsetof(gentity_s, s.loopSound) == 164, "");
        static_assert(offsetof(gentity_s, s.surfType) == 168, "");
        static_assert(offsetof(gentity_s, s.index) == 172, "");
        static_assert(offsetof(gentity_s, s.clientNum) == 176, "");
        static_assert(offsetof(gentity_s, s.iHeadIcon) == 180, "");
        static_assert(offsetof(gentity_s, s.iHeadIconTeam) == 184, "");
        static_assert(offsetof(gentity_s, s.solid) == 188, "");
        static_assert(offsetof(gentity_s, s.eventParm) == 192, "");
        static_assert(offsetof(gentity_s, s.eventSequence) == 196, "");
        static_assert(offsetof(gentity_s, s.events) == 200, "");
        static_assert(offsetof(gentity_s, s.eventParms) == 216, "");
        static_assert(offsetof(gentity_s, s.weapon) == 232, "");
        static_assert(offsetof(gentity_s, s.weaponModel) == 236, "");
        static_assert(offsetof(gentity_s, s.targetName) == 240, "");
        static_assert(offsetof(gentity_s, s.partBits) == 268, "");

        static_assert(offsetof(gentity_s, r) == 284, "");
        static_assert(offsetof(gentity_s, r.linked) == 284, "");
        static_assert(offsetof(gentity_s, r.bmodel) == 285, "");
        static_assert(offsetof(gentity_s, r.svFlags) == 286, "");
        static_assert(offsetof(gentity_s, r.clientMask) == 288, "");
        static_assert(offsetof(gentity_s, r.absmin) == 296, "");
        static_assert(offsetof(gentity_s, r.absmax) == 308, "");
        static_assert(offsetof(gentity_s, r.contents) == 328, "");
        static_assert(offsetof(gentity_s, r.currentOrigin) == 356, "");
        static_assert(offsetof(gentity_s, r.currentAngles) == 368, "");

        static_assert(offsetof(gentity_s, client) == 388, "");
        static_assert(offsetof(gentity_s, model) == 412, "");
        static_assert(offsetof(gentity_s, classname) == 420, "");
        static_assert(offsetof(gentity_s, target) == 422, "");
        static_assert(offsetof(gentity_s, targetname) == 424, "");
        static_assert(offsetof(gentity_s, script_noteworthy) == 426, "");

        static_assert(offsetof(gentity_s, spawnflags) == 432, "");
        static_assert(offsetof(gentity_s, flags) == 436, "");

        static_assert(offsetof(gentity_s, health) == 468, "");
        static_assert(offsetof(gentity_s, maxHealth) == 472, "");
        static_assert(offsetof(gentity_s, dmg) == 476, "");
        static_assert(offsetof(gentity_s, birthtime) == 812, "");

        struct level_locals_t
        {
            gclient_s *clients;
            gentity_s *gentities;
            int gentitySize;
            int num_entities;
            char pad_0004[556];
            int time;
        };
        static_assert(offsetof(level_locals_t, clients) == 0x0000, "");
        static_assert(offsetof(level_locals_t, time) == 572, "");

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

        struct ent_field_t
        {
            const char *name;
            int ofs;
            fieldtype_t type;
            void (*callback)(gentity_s *, int);
        };

#pragma warning(disable : 4324)
        struct __declspec(align(8)) scr_entref_t
        {
            uint16_t entnum;
            uint16_t classnum;
            uint16_t localclientnum;
        };
#pragma warning(default : 4324)
        static_assert(sizeof(scr_entref_t) == 8, "");
        static_assert(offsetof(scr_entref_t, entnum) == 0x0, "");
        static_assert(offsetof(scr_entref_t, classnum) == 0x2, "");
        static_assert(offsetof(scr_entref_t, localclientnum) == 0x4, "");

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

        typedef void (*BuiltinMethod)(scr_entref_t entref);

        struct BuiltinMethodDef
        {
            const char *actionString;
            BuiltinMethod actionFunc;
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

        // usercmd_t->button bits
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

        struct usercmd_s
        {
            int serverTime;                  // OFS: 0x00 SIZE: 0x4
            button_mask buttons;             // OFS: 0x04 SIZE: 0x4
            int angles[3];                   // OFS: 0x08 SIZE: 0xC
            unsigned __int8 weapon;          // OFS: 0x14 SIZE: 0x1
            unsigned __int8 offHandIndex;    // OFS: 0x15 SIZE: 0x1
            char forwardmove;                // OFS: 0x16 SIZE: 0x1
            char rightmove;                  // OFS: 0x17 SIZE: 0x1
            char pad[4];                     //
            float meleeChargeYaw;            // OFS: 0x1C SIZE: 0x4
            unsigned __int8 meleeChargeDist; // OFS: 0x20 SIZE: 0x1
            char pad2[7];                    //
            char selectedLocation[2];        // OFS: 0x28 SIZE: 0x2
        };
        static_assert(sizeof(usercmd_s) == 44, "");
        static_assert(offsetof(usercmd_s, serverTime) == 0x0, "");
        static_assert(offsetof(usercmd_s, buttons) == 0x4, "");
        static_assert(offsetof(usercmd_s, angles) == 0x8, "");
        static_assert(offsetof(usercmd_s, weapon) == 0x14, "");
        static_assert(offsetof(usercmd_s, offHandIndex) == 0x15, "");
        static_assert(offsetof(usercmd_s, forwardmove) == 0x16, "");
        static_assert(offsetof(usercmd_s, rightmove) == 0x17, "");
        static_assert(offsetof(usercmd_s, meleeChargeYaw) == 0x1C, "");
        static_assert(offsetof(usercmd_s, meleeChargeDist) == 0x20, "");
        static_assert(offsetof(usercmd_s, selectedLocation) == 0x28, "");

        struct clientHeader_t
        {
            int state;        // OFS: 0x0 SIZE: 0x4
            int sendAsActive; // OFS: 0x4 SIZE: 0x4
            int deltaMessage; // OFS: 0x8 SIZE: 0x4
        };
        static_assert(offsetof(clientHeader_t, state) == 0x0, "");
        static_assert(offsetof(clientHeader_t, sendAsActive) == 0x4, "");
        static_assert(offsetof(clientHeader_t, deltaMessage) == 0x8, "");

        struct client_t
        {
            clientHeader_t header; // OFS: 0x0
            char pad[134888];      //
            usercmd_s lastUsercmd; // OFS: 0x20EF4 SIZE: 0x2C
            char pad1[1032];       //
            char name[32];         // OFS: 0x21328 SIZE: 0x20
            char pad2[172];        //
            gentity_s *gentity;    // OFS: 0x213F4 SIZE: 0x4
            char pad3[12];         //
            int nextSnapshotTime;  // OFS: 0x21404 SIZE: 0x4
            char pad4[614948];     // padding to reach end of struct
        };

        static_assert(sizeof(client_t) == 751148, "");
        static_assert(offsetof(client_t, header) == 0x0, "");
        static_assert(offsetof(client_t, lastUsercmd) == 0x20EF4, "");
        static_assert(offsetof(client_t, name) == 0x21328, "");
        static_assert(offsetof(client_t, gentity) == 0x213F4, "");
        static_assert(offsetof(client_t, nextSnapshotTime) == 0x21404, "");

        struct serverStaticHeader_t
        {
            client_t *clients; // OFS: 0x0 SIZE: 0x4
            int time;          // OFS: 0x4 SIZE: 0x4
        };

        static_assert(offsetof(serverStaticHeader_t, clients) == 0x0, "");
        static_assert(offsetof(serverStaticHeader_t, time) == 0x4, "");

        struct viewState_t;
        struct weaponState_t;
        struct pmove_t;

        struct scrVarPub_t
        {
            const char *fieldBuffer;
            unsigned __int16 canonicalStrCount;
            bool developer;
            bool developer_script;
            bool evaluate;
            const char *error_message; // OFS: 0xC SIZE: 0x4
        };
        static_assert(offsetof(scrVarPub_t, error_message) == 0xC, "");

        struct __declspec(align(4)) NetField
        {
            char *name;                  // OFS: 0x0 SIZE: 0x4
            int offset;                  // OFS: 0x4 SIZE: 0x4
            int bits;                    // OFS: 0x8 SIZE: 0x4
            unsigned __int8 changeHints; // OFS: 0xC SIZE: 0x1
        };
        static_assert(sizeof(NetField) == 0x10, "");
        static_assert(offsetof(NetField, name) == 0x0, "");
        static_assert(offsetof(NetField, offset) == 0x4, "");
        static_assert(offsetof(NetField, bits) == 0x8, "");
        static_assert(offsetof(NetField, changeHints) == 0xC, "");

        struct SpawnFuncEntry
        {
            const char *classname;         // OFS: 0x0 SIZE: 0x4
            void (*callback)(gentity_s *); // OFS: 0x4 SIZE: 0x4
        };
        static_assert(sizeof(SpawnFuncEntry) == 0x8, "");
        static_assert(offsetof(SpawnFuncEntry, classname) == 0x0, "");
        static_assert(offsetof(SpawnFuncEntry, callback) == 0x4, "");
    }
}
