#include "g_client_fields.h"
#include "common.h"

namespace iw4
{
    namespace sp
    {

        // Bitmask for client flags
        namespace ClientFlags
        {
            const int CF_BIT_NOCLIP = (1 << 0);
            const int CF_BIT_UFO = (1 << 1);
            // CF_BIT_NOCLIP = (1 << 0),
            // CF_BIT_UFO = (1 << 1),
            // CF_BIT_FROZEN = (1 << 2),
            // CF_BIT_DISABLE_USABILITY = (1 << 3),
            // CF_BIT_NO_KNOCKBACK = (1 << 4),
        }

        // Bitmask for entity flags
        namespace EntityFlags
        {
            const int FL_GODMODE = 1 << 0;
            // FL_DEMI_GODMODE = 1 << 1,
            // FL_NOTARGET = 1 << 2,
            // FL_NO_KNOCKBACK = 1 << 3,
            // FL_NO_RADIUS_DAMAGE = 1 << 4,
            // FL_SUPPORTS_LINKTO = 1 << 12,
            // FL_NO_AUTO_ANIM_UPDATE = 1 << 13,
            // FL_GRENADE_TOUCH_DAMAGE = 1 << 14,
            // FL_STABLE_MISSILES = 1 << 17,
            // FL_REPEAT_ANIM_UPDATE = 1 << 18,
            // FL_VEHICLE_TARGET = 1 << 19,
            // FL_GROUND_ENT = 1 << 20,
            // FL_CURSOR_HINT = 1 << 21,
            // FL_MISSILE_ATTRACTOR = 1 << 23,
            // FL_WEAPON_BEING_GRABBED = 1 << 24,
            // FL_DELETE = 1 << 25,
            // FL_BOUNCE = 1 << 26,
            // FL_MOVER_SLIDE = 1 << 27
        }

        const unsigned short CLIENT_FIELD_MASK = 0x6000;

        void ClientScr_SetGod(gclient_s *pSelf, const client_fields_s *field)
        {
            auto ent = &g_entities[pSelf - level->clients];
            auto p_flags = &ent->flags;

            const auto value = Scr_GetInt(0);
            if (value)
            {
                *p_flags |= EntityFlags::FL_GODMODE;
            }
            else
            {
                *p_flags &= ~EntityFlags::FL_GODMODE;
            }
        }

        void ClientScr_GetGod(gclient_s *client, const client_fields_s *field)
        {
            const auto ent = &g_entities[client - level->clients];
            const auto p_flags = &ent->flags;
            const int value = (*p_flags & EntityFlags::FL_GODMODE) ? 1 : 0;
            Scr_AddInt(value);
        }

        void ClientScr_SetNoclip(gclient_s *pSelf, const client_fields_s *field)
        {
            auto ent = &g_entities[pSelf - level->clients];
            auto p_flags = &ent->client->flags;

            const auto value = Scr_GetInt(0);
            if (value)
            {
                *p_flags |= ClientFlags::CF_BIT_NOCLIP;
            }
            else
            {
                *p_flags &= ~ClientFlags::CF_BIT_NOCLIP;
            }
        }

        void ClientScr_GetNoclip(gclient_s *client, const client_fields_s *field)
        {
            const auto ent = &g_entities[client - level->clients];
            const auto p_flags = &ent->client->flags;
            const int value = (*p_flags & ClientFlags::CF_BIT_NOCLIP) ? 1 : 0;
            Scr_AddInt(value);
        }

        void ClientScr_SetUFO(gclient_s *pSelf, const client_fields_s *field)
        {
            auto ent = &g_entities[pSelf - level->clients];
            auto p_flags = &ent->client->flags;

            const auto value = Scr_GetInt(0);
            if (value)
            {
                *p_flags |= ClientFlags::CF_BIT_UFO;
            }
            else
            {
                *p_flags &= ~ClientFlags::CF_BIT_UFO;
            }
        }

        void ClientScr_GetUFO(gclient_s *client, const client_fields_s *field)
        {
            const auto ent = &g_entities[client - level->clients];
            const auto p_flags = &ent->client->flags;
            const int value = (*p_flags & ClientFlags::CF_BIT_UFO) ? 1 : 0;
            Scr_AddInt(value);
        }

        const client_fields_s fields_extended[] =
            {
                // Original fields
                {"playername", 0, F_CSTRING, ClientScr_ReadOnly, ClientScr_GetName},
                {"deathinvulnerabletime", 44564, F_INT, NULL, NULL},
                {"criticalbulletdamagedist", 44560, F_FLOAT, NULL, NULL},
                {"attackercount", 44575, F_BYTE, NULL, NULL},
                {"damagemultiplier", 44584, F_FLOAT, NULL, NULL},
                {"laststand", 44996, F_INT, NULL, NULL},
                {"motiontrackerenabled", 44336, F_INT, NULL, NULL},
                // Added fields
                {"god", 0, F_INT, ClientScr_SetGod, ClientScr_GetGod},
                {"noclip", 0, F_INT, ClientScr_SetNoclip, ClientScr_GetNoclip},
                {"ufo", 0, F_INT, ClientScr_SetUFO, ClientScr_GetUFO},
                {NULL, 0, F_INT, NULL, NULL}};

        Detour GScr_AddFieldsForClient_Detour;

        void GScr_AddFieldsForClient_Hook()
        {
            for (int i = 0; fields_extended[i].name != nullptr; i++)
            {
                Scr_AddClassField(0, fields_extended[i].name, static_cast<unsigned short>(i) | CLIENT_FIELD_MASK);
                DbgPrint("Added client field: %s\n", fields_extended[i].name);
            }
        }

        Detour Scr_SetClientField_Detour;

        void Scr_SetClientField_Hook(gclient_s *client, unsigned int offset)
        {
            const auto field = &fields_extended[offset];
            if (field->setter)
                field->setter(client, field);
            else
                Scr_SetGenericField(reinterpret_cast<unsigned char *>(client), field->type, field->ofs);
        }

        void Scr_GetClientField(gclient_s *client, unsigned int offset)
        {
            const auto field = &fields_extended[offset];
            if (field->getter)
                field->getter(client, field);
            else
                Scr_GetGenericField(reinterpret_cast<unsigned char *>(client), field->type, field->ofs);
        }

        // Retail TU9 SP has Scr_GetClientField and Scr_GetEntityField inlined, so we need to hook Scr_GetObjectField
        // to handle client fields.
        Detour Scr_GetObjectField_Detour;

        void Scr_GetObjectField_Hook(unsigned int classnum, int entnum, int offset)
        {
            // Mimic the client field conditions
            if (classnum == 0 && (offset & CLIENT_FIELD_MASK) == CLIENT_FIELD_MASK)
            {
                // This is a client field
                Scr_GetClientField(&g_clients[entnum], offset & ~CLIENT_FIELD_MASK);
                return;
            }
            else
            {
                // Call the original function for non-client fields
                Scr_GetObjectField_Detour.GetOriginal<decltype(Scr_GetObjectField)>()(classnum, entnum, offset);
            }
        }

        g_client_fields::g_client_fields()
        {
            GScr_AddFieldsForClient_Detour = Detour(GScr_AddFieldsForClient, GScr_AddFieldsForClient_Hook);
            GScr_AddFieldsForClient_Detour.Install();

            Scr_SetClientField_Detour = Detour(Scr_SetClientField, Scr_SetClientField_Hook);
            Scr_SetClientField_Detour.Install();

            Scr_GetObjectField_Detour = Detour(Scr_GetObjectField, Scr_GetObjectField_Hook);
            Scr_GetObjectField_Detour.Install();
        }

        g_client_fields::~g_client_fields()
        {
            GScr_AddFieldsForClient_Detour.Remove();

            Scr_SetClientField_Detour.Remove();

            Scr_GetObjectField_Detour.Remove();
        }
    }
}
