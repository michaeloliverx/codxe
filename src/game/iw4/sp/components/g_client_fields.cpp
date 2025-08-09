#include "g_client_fields.h"
#include "common.h"

namespace iw4
{
namespace sp
{
const unsigned short CLIENT_FIELD_MASK = 0x6000;

void ClientScr_SetEntityFlags(gclient_s *pSelf, const client_fields_s *pField)
{
    gentity_s *ent = &g_entities[pSelf - level->clients];
    ent->flags = Scr_GetInt(0);
}

void ClientScr_GetEntityFlags(gclient_s *pSelf, const client_fields_s *field)
{
    const gentity_s *ent = &g_entities[pSelf - level->clients];
    Scr_AddInt(ent->flags);
}

const client_fields_s fields_extended[] = {
    // Original fields
    {"playername", 0, F_CSTRING, ClientScr_ReadOnly, ClientScr_GetName},
    {"deathinvulnerabletime", 44564, F_INT, NULL, NULL},
    {"criticalbulletdamagedist", 44560, F_FLOAT, NULL, NULL},
    {"attackercount", 44575, F_BYTE, NULL, NULL},
    {"damagemultiplier", 44584, F_FLOAT, NULL, NULL},
    {"laststand", 44996, F_INT, NULL, NULL},
    {"motiontrackerenabled", 44336, F_INT, NULL, NULL},

    // // Added fields
    {"clientflags", offsetof(gclient_s, flags), F_INT, NULL, NULL},
    {"entityflags", NULL, F_INT, ClientScr_SetEntityFlags, ClientScr_GetEntityFlags},

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
} // namespace sp
} // namespace iw4
