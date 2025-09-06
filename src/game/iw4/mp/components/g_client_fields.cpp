#include "g_client_fields.h"
#include "common.h"

namespace iw4
{
namespace mp
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

typedef void (*ClientFieldFunc)(gclient_s *, const client_fields_s *);

const client_fields_s fields_extended[] = {
    // Built-in fields
    {"name", 0, F_CSTRING, (ClientFieldFunc)0x8221B208, (ClientFieldFunc)0x8221B1A0},
    {"sessionteam", 0, F_STRING, (ClientFieldFunc)0x8221A8C0, (ClientFieldFunc)0x8221A9E0},
    {"sessionstate", 0, F_STRING, (ClientFieldFunc)0x8221AA48, (ClientFieldFunc)0x8221AB50},
    {"maxhealth", 12840, F_INT, (ClientFieldFunc)0x8221ABB8, NULL},
    {"score", 12696, F_INT, (ClientFieldFunc)0x8221AC50, NULL},
    {"deaths", 12700, F_INT, NULL, NULL},
    {"statusicon", 0, F_STRING, (ClientFieldFunc)0x8221ADA8, (ClientFieldFunc)0x8221ADE0},
    {"headicon", 0, F_STRING, (ClientFieldFunc)0x8221AE40, (ClientFieldFunc)0x8221AEA0},
    {"headiconteam", 0, F_STRING, (ClientFieldFunc)0x8221AF30, (ClientFieldFunc)0x8221B050},
    {"kills", 12704, F_INT, NULL, NULL},
    {"assists", 12708, F_INT, NULL, NULL},
    {"hasradar", 13028, F_INT, NULL, NULL},
    {"isradarblocked", 13032, F_INT, NULL, NULL},
    {"radarmode", 0, F_STRING, (ClientFieldFunc)0x8221A7D0, (ClientFieldFunc)0x8221A888},
    {"forcespectatorclient", 12676, F_INT, (ClientFieldFunc)0x8221AC88, NULL},
    {"killcamentity", 12680, F_INT, (ClientFieldFunc)0x8221ACE8, NULL},
    {"killcamentitylookat", 12684, F_INT, (ClientFieldFunc)0x8221AD48, NULL},
    {"archivetime", 12692, F_FLOAT, (ClientFieldFunc)0x8221B0D0, (ClientFieldFunc)0x8221B118},
    {"psoffsettime", 13024, F_INT, (ClientFieldFunc)0x8221B160, (ClientFieldFunc)0x8221B198},
    {"pers", 12712, F_OBJECT, (ClientFieldFunc)0x8221B208, NULL},

    // Added fields
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

// Retail TU MP has Scr_GetClientField and Scr_GetEntityField inlined, so we need to hook further up Scr_GetObjectField
// to handle client fields.
Detour Scr_GetObjectField_Detour;

void Scr_GetObjectField_Hook(unsigned int classnum, int entnum, int offset)
{
    // Mimic the client field conditions
    if (classnum == 0 && (offset & CLIENT_FIELD_MASK) == CLIENT_FIELD_MASK)
    {
        // This is a client field
        Scr_GetClientField(&level->clients[entnum], offset & ~CLIENT_FIELD_MASK);
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
} // namespace mp
} // namespace iw4
