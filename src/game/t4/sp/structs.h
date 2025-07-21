#pragma once

namespace t4
{
    namespace sp
    {
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

        enum entityType_t : __int32
        {
            ET_GENERAL,
            ET_PLAYER,
            ET_PLAYER_CORPSE,
            ET_ITEM,
            ET_MISSILE,
            ET_INVISIBLE,
            ET_SCRIPTMOVER,
            ET_SOUND_BLEND,
            ET_FX,
            ET_LOOP_FX,
            ET_PRIMARY_LIGHT,
            ET_MG42,
            ET_PLANE,
            ET_VEHICLE,
            ET_VEHICLE_CORPSE,
            ET_VEHICLE_COLLMAP,
            ET_ACTOR,
            ET_ACTOR_SPAWNER,
            ET_ACTOR_CORPSE
        };

        struct gclient_s
        {
            char pad_0[8780];
            int buttons; // Offset 8780
            int oldbuttons;
            int latched_buttons;       // Offset 8784
            int buttonsSinceLastFrame; // Offset 8792
            char pad_1[240];
        };
        static_assert(sizeof(gclient_s) == 9036, "");
        static_assert(offsetof(gclient_s, buttons) == 8780, "");
        static_assert(offsetof(gclient_s, buttonsSinceLastFrame) == 8792, "");

        struct EntHandle
        {
            unsigned __int16 number;
            unsigned __int16 infoIndex;
        };

        struct gentity_s
        {
            struct entityState_s
            {
                int number;
                int eType;
                char pad_0[160];
                int index; // OFS: 168
                char pad_00[108];
            } s;

            struct entityShared_t
            {
                unsigned __int8 linked;
                unsigned __int8 bmodel;
                unsigned __int8 svFlags;
                int clientMask[2];
                unsigned __int8 inuse;
                int broadcastTime;
                float mins[3];
                float maxs[3];
                int contents;
                float absmin[3];
                float absmax[3];
                float currentOrigin[3];
                float currentAngles[3];
                EntHandle ownerNum;
                int eventTime;
            } r;

            gclient_s *client;          // OFS: 384 SIZE: 0x4
            char pad_0[28];             //
            unsigned __int16 classname; // OFS: 416 SIZE: 0x2
            char pad_1[18];             //
            int flags;                  // OFS: 436 SIZE: 0x4
            char pad_2[448];            //
        };
        static_assert(sizeof(gentity_s) == 888, "");
        static_assert(offsetof(gentity_s, s.index) == 168, "");

        static_assert(offsetof(gentity_s, r.bmodel) == 281, "");
        static_assert(offsetof(gentity_s, r.contents) == 324, "");
        static_assert(offsetof(gentity_s, client) == 384, "");
        static_assert(offsetof(gentity_s, classname) == 416, "");
        static_assert(offsetof(gentity_s, flags) == 436, "");

        struct usercmd_s
        {
            int serverTime;   // 0x00
            int buttons;      // 0x04
            char pad[14];     //
            char forwardmove; // OFS: 0x16 SIZE: 0x1
            char rightmove;   // OFS: 0x17 SIZE: 0x1
            char pad2[32];    // 0x08
        };
        static_assert(sizeof(usercmd_s) == 56, "");
        static_assert(offsetof(usercmd_s, serverTime) == 0x00, "");
        static_assert(offsetof(usercmd_s, buttons) == 4, "");
        static_assert(offsetof(usercmd_s, forwardmove) == 0x16, "");
        static_assert(offsetof(usercmd_s, rightmove) == 0x17, "");

        struct __declspec(align(4)) client_t
        {
            char pad[69880];
            usercmd_s lastUsercmd;
            char pad2[283340];
        };
        static_assert(sizeof(client_t) == 353276, "");
        static_assert(offsetof(client_t, lastUsercmd) == 69880, "");

        enum scriptInstance_t : __int32
        {
            SCRIPTINSTANCE_SERVER = 0x0,
            SCRIPTINSTANCE_CLIENT = 0x1,
            SCRIPT_INSTANCE_MAX = 0x2,
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

        struct SpawnFuncEntry
        {
            const char *classname;
            void (*callback)(gentity_s *);
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

        struct client_fields_s
        {
            const char *name;
            int ofs;
            fieldtype_t type;
            unsigned int whichbits;
            void (*setter)(gclient_s *pSelf, const client_fields_s *pField);
            void (*getter)(gclient_s *pSelf, const client_fields_s *pField);
        };

        struct level_locals_t
        {
            gclient_s *clients;
            gentity_s *gentities;
            int num_entities;
        };
        static_assert(offsetof(level_locals_t, clients) == 0x0, "");
        static_assert(offsetof(level_locals_t, gentities) == 0x4, "");

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

        struct MapEnts
        {
            const char *name;   // OFS: 0x0 SIZE: 0x4
            char *entityString; // OFS: 0x4 SIZE: 0x4
            int numEntityChars; // OFS: 0x8 SIZE: 0x4
        };
        static_assert(sizeof(MapEnts) == 12, "");
        static_assert(offsetof(MapEnts, name) == 0x0, "");
        static_assert(offsetof(MapEnts, entityString) == 0x4, "");

        struct clipMap_t
        {
            const char *name;
            int isInUse;
            char pad[140];
            int numSubModels;
            cmodel_t *cmodels;
            char pad2[24];
            MapEnts *mapEnts; // OFS: 180 SIZE: 0xC
        };
        static_assert(offsetof(clipMap_t, numSubModels) == 148, "");
        static_assert(offsetof(clipMap_t, cmodels) == 152, "");
        static_assert(offsetof(clipMap_t, mapEnts) == 180, "");
    }
}
