#pragma once

#pragma warning(disable : 4480)

namespace iw4
{
    namespace sp
    {
        struct playerState_s
        {
            char pad[40];
            float velocity[3];
            char pad1[44144];
        };
        static_assert(sizeof(playerState_s) == 44196, "");
        static_assert(offsetof(playerState_s, velocity) == 40, "");

        struct gclient_s
        {
            playerState_s ps;
            char pad0[176];
            int flags;
            char pad1[640];
        };
        static_assert(sizeof(gclient_s) == 45016, "");
        static_assert(offsetof(gclient_s, ps) == 0x0, "");
        static_assert(offsetof(gclient_s, ps.velocity) == 40, "");
        static_assert(offsetof(gclient_s, flags) == 44372, "");

        struct gentity_s
        {
            char pad0[264];
            gclient_s *client;
            char pad1[44];
            int flags;
            char pad2[308];
        };
        static_assert(sizeof(gentity_s) == 624, "");
        static_assert(offsetof(gentity_s, client) == 264, "");
        static_assert(offsetof(gentity_s, flags) == 312, "");

        struct sentient_s;
        struct actor_s;
        struct Vehicle;
        struct Turret;

        struct level_locals_t
        {
            gclient_s *clients;
            gentity_s *gentities;
            int num_entities;
            gentity_s *firstFreeEnt;
            gentity_s *lastFreeEnt;
            sentient_s *sentients;
            actor_s *actors;
            Vehicle *vehicles;
            Turret *turrets;
            int initializing;
            int clientIsSpawning;
            int maxclients;
        };
        static_assert(offsetof(level_locals_t, clients) == 0x0, "");
        static_assert(offsetof(level_locals_t, gentities) == 0x4, "");
        static_assert(offsetof(level_locals_t, maxclients) == 44, "");

        struct weaponParms
        {
            float forward[3];
            float right[3];
            float up[3];
            float muzzleTrace[3];
            float gunForward[3];
            unsigned int weaponIndex;
            const struct WeaponDef *weapDef;
            const struct WeaponCompleteDef *weapCompleteDef;
        };
        static_assert(sizeof(weaponParms) == 0x48, "");

        struct scr_entref_t
        {
            unsigned __int16 entnum;
            unsigned __int16 classnum;
        };

        typedef void (*BuiltinMethod)(scr_entref_t);

        struct BuiltinMethodDef
        {
            const char *actionString;
            void (*actionFunc)(scr_entref_t);
            int type;
        };

        enum fieldtype_t : __int32
        {
            F_INT = 0x0,
            F_SHORT = 0x1,
            F_BYTE = 0x2,
            F_FLOAT = 0x3,
            F_STRING = 0x4,
            F_CSTRING = 0x5,
            F_VECTOR = 0x6,
            F_ENTITY = 0x7,
            F_ENTHANDLE = 0x8,
            F_ACTOR = 0x9,
            F_SENTIENT = 0xA,
            F_SENTIENTHANDLE = 0xB,
            F_CLIENT = 0xC,
            F_PATHNODE = 0xD,
            F_ANGLES_YAW = 0xE,
            F_MODEL = 0xF,
            F_ACTORGROUP = 0x10,
        };

        struct client_fields_s
        {
            const char *name;
            int ofs;
            fieldtype_t type;
            void (*setter)(gclient_s *, const client_fields_s *);
            void (*getter)(gclient_s *, const client_fields_s *);
        };
    }
}
