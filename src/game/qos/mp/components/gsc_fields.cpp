#include "pch.h"
#include "gsc_fields.h"
#include "unordered_map"

const unsigned int CLIENT_FIELD_MASK = 0xC000;

const static std::uint16_t CLIENT_FIELD_OFFSET_START = 16; // retail client fields count
const static std::uint16_t ENTITY_FIELD_OFFSET_START = 32; // retail entity fields count

namespace qos
{
namespace mp
{

void ClientScr_SetEntityFlags(gclient_s *pSelf, const client_fields_s *pField)
{
    gentity_s *ent = &g_entities[pSelf - *level_clients];
    ent->flags = Scr_GetInt(0);
}

void ClientScr_GetEntityFlags(gclient_s *pSelf, const client_fields_s *field)
{
    const gentity_s *ent = &g_entities[pSelf - *level_clients];
    Scr_AddInt(ent->flags);
}

client_fields_s custom_client_fields[] = {
    {"noclip", offsetof(gclient_s, noclip), F_INT, nullptr, nullptr},
    {"ufo", offsetof(gclient_s, ufo), F_INT, nullptr, nullptr},

    // this should be done on the entity fields but cba
    {"entityflags", NULL, F_INT, ClientScr_SetEntityFlags, ClientScr_GetEntityFlags},
};

Detour GScr_AddFieldsForClient_Detour;
void GScr_AddFieldsForClient_Hook()
{
    GScr_AddFieldsForClient_Detour.GetOriginal<GScr_AddFieldsForClient_t>()();

    for (unsigned int i = 0; i < ARRAYSIZE(custom_client_fields); i++)
    {
        client_fields_s *field = &custom_client_fields[i];
        const auto offset = CLIENT_FIELD_OFFSET_START + i;
        Scr_AddClassField(CLASS_NUM_ENTITY, field->name, (unsigned short)(offset) | CLIENT_FIELD_MASK);
    }
}

Detour Scr_SetClientField_Detour;
void Scr_SetClientField_Stub(gclient_s *client, int offset)
{
    const client_fields_s *f;
    if (offset < CLIENT_FIELD_OFFSET_START)
    {
        f = &client_fields[offset];
    }
    else
    {
        f = &custom_client_fields[offset - CLIENT_FIELD_OFFSET_START];
    }

    if (f->setter)
    {
        f->setter(client, f);
    }
    else
    {
        Scr_SetGenericField((unsigned __int8 *)client, f->type, f->ofs);
    }
}

void Scr_GetClientField(gclient_s *client, int offset)
{
    const client_fields_s *f;
    if (offset < CLIENT_FIELD_OFFSET_START)
    {
        f = &client_fields[offset];
    }
    else
    {
        f = &custom_client_fields[offset - CLIENT_FIELD_OFFSET_START];
    }

    if (f->getter)
    {
        f->getter(client, f);
    }
    else
    {
        Scr_GetGenericField((unsigned __int8 *)client, f->type, f->ofs);
    }
}

Detour Scr_GetObjectField_Detour;
void Scr_GetObjectField_Hook(unsigned int classnum, unsigned int entnum, unsigned int offset)
{
    // client field
    if (classnum == CLASS_NUM_ENTITY && (offset & CLIENT_FIELD_MASK) == CLIENT_FIELD_MASK)
    {
        Scr_GetClientField(g_entities[entnum].client, offset & 0x3FFF);
    }
    else
    {
        Scr_GetObjectField_Detour.GetOriginal<Scr_GetObjectField_t>()(classnum, entnum, offset);
    }
}

GSCFields::GSCFields()
{
    GScr_AddFieldsForClient_Detour = Detour(GScr_AddFieldsForClient, GScr_AddFieldsForClient_Hook);
    GScr_AddFieldsForClient_Detour.Install();

    Scr_SetClientField_Detour = Detour(Scr_SetClientField, Scr_SetClientField_Stub);
    Scr_SetClientField_Detour.Install();

    Scr_GetObjectField_Detour = Detour(Scr_GetObjectField, Scr_GetObjectField_Hook);
    Scr_GetObjectField_Detour.Install();
}

GSCFields::~GSCFields()
{
    GScr_AddFieldsForClient_Detour.Remove();
    Scr_SetClientField_Detour.Remove();
    Scr_GetObjectField_Detour.Remove();
}
} // namespace mp
} // namespace qos
