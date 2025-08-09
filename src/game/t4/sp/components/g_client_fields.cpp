#include "g_client_fields.h"
#include "common.h"

namespace t4
{
namespace sp
{
const int CLIENT_FIELD_MASK = 0x6000;

void ClientScr_SetGod(gclient_s *pSelf, const client_fields_s *field)
{
    auto ent = &g_entities[pSelf - g_clients];

    const auto value = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if (value)
    {
        ent->flags |= 1; // Set the god mode flag
    }
    else
    {
        ent->flags &= ~1; // Clear the god mode flag
    }
}

void ClientScr_GetGod(gclient_s *pSelf, const client_fields_s *field)
{
    const auto ent = &g_entities[pSelf - g_clients];
    const int value = (ent->flags & 1) ? 1 : 0;
    Scr_AddInt(value, SCRIPTINSTANCE_SERVER);
}

// Updated fields array with named function pointers
const client_fields_s fields_extended[] = {
    // Original fields
    {"playername", 0, F_LSTRING, 0x0, ClientScr_ReadOnly, ClientScr_GetName},
    {"downs", 8392, F_INT, 0x0, NULL, NULL},
    {"revives", 8396, F_INT, 0x0, NULL, NULL},
    {"headshots", 8400, F_INT, 0x0, NULL, NULL},
    {"nolean", 12, F_BITFLAG, 0x2000000, NULL, NULL},
    {"nopronerotation", 12, F_BITFLAG, 0x10000000, NULL, NULL},
    {"kills", 8384, F_INT, 0x0, NULL, NULL},
    {"assists", 8388, F_INT, 0x0, NULL, NULL},
    {"score", 8380, F_INT, 0x0, ClientScr_SetScore, NULL},
    {"sessionstate", 0, F_STRING, 0x0, ClientScr_SetSessionState, ClientScr_GetSessionState},
    {"statusicon", 0, F_STRING, 0x0, ClientScr_SetStatusIcon, ClientScr_GetStatusIcon},
    {"spectatorclient", 8368, F_INT, 0x0, ClientScr_SetSpectatorClient, NULL},
    {"archivetime", 8376, F_FLOAT, 0x0, ClientScr_SetArchiveTime, ClientScr_GetArchiveTime},
    {"psoffsettime", 8760, F_INT, 0x0, ClientScr_SetPSOffsetTime, ClientScr_GetPSOffsetTime},
    {"pers", 8412, F_OBJECT, 0x0, ClientScr_ReadOnly, NULL},
    {"usingturret", 204, F_BITFLAG, 0x300, ClientScr_ReadOnly, NULL},
    {"usingvehicle", 204, F_BITFLAG, 0x4000, ClientScr_ReadOnly, NULL},
    {"vehicleposition", 2112, F_INT, 0x0, ClientScr_ReadOnly, NULL},
    {"headicon", 0, F_STRING, 0x0, ClientScr_SetHeadIcon, ClientScr_GetHeadIcon},

    // Custom fields
    {"noclip", 8768, F_INT, 0x0, NULL, NULL},
    {"ufo", 8772, F_INT, 0x0, NULL, NULL},
    {"god", 0, F_INT, 0x0, ClientScr_SetGod, ClientScr_GetGod},
    {NULL, 0, F_INT, 0x0, NULL, NULL}};

Detour GScr_AddFieldsForClient_Detour;

void GScr_AddFieldsForClient_Hook()
{
    for (int i = 0; fields_extended[i].name != nullptr; i++)
    {
        Scr_AddClassField(0, fields_extended[i].name, static_cast<unsigned short>(i) | CLIENT_FIELD_MASK,
                          SCRIPTINSTANCE_SERVER);
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
        Scr_SetGenericField(reinterpret_cast<unsigned char *>(client), field->type, field->ofs, SCRIPTINSTANCE_SERVER);
}

void Scr_GetClientField(gclient_s *client, unsigned int offset)
{
    const auto field = &fields_extended[offset];
    if (field->getter)
        field->getter(client, field);
    else
        Scr_GetGenericField(reinterpret_cast<unsigned char *>(client), field->type, field->ofs, SCRIPTINSTANCE_SERVER,
                            field->whichbits);
}

// Retail TU7 SP has Scr_GetClientField and Scr_GetEntityField inlined, so we need to hook Scr_GetObjectField
// to handle client fields.
Detour Scr_GetObjectField_Detour;

void Scr_GetObjectField_Hook(unsigned int classnum, int entnum, int offset, scriptInstance_t inst)
{
    // Mimic the client field conditions
    if (classnum == 0 && (offset & CLIENT_FIELD_MASK) == CLIENT_FIELD_MASK)
    {
        // This is a client field
        Scr_GetClientField(g_entities[entnum].client, offset & ~CLIENT_FIELD_MASK);
        return;
    }
    else
    {
        // Call the original function for non-client fields
        Scr_GetObjectField_Detour.GetOriginal<decltype(Scr_GetObjectField)>()(classnum, entnum, offset, inst);
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
} // namespace t4
