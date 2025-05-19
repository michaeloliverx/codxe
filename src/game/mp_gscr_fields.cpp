#include "common.h"
#include "mp_gscr_fields.h"

const unsigned int CLIENT_FIELD_MASK = 0xC000;

namespace mp
{
    ClientScr_ReadOnly_t ClientScr_ReadOnly = reinterpret_cast<ClientScr_ReadOnly_t>(0x82366748);
    ClientScr_GetName_t ClientScr_GetName = reinterpret_cast<ClientScr_GetName_t>(0x82366C28);
    ClientScr_SetSessionTeam_t ClientScr_SetSessionTeam = reinterpret_cast<ClientScr_SetSessionTeam_t>(0x82366EC0);
    ClientScr_GetSessionTeam_t ClientScr_GetSessionTeam = reinterpret_cast<ClientScr_GetSessionTeam_t>(0x82366BB0);
    ClientScr_SetSessionState_t ClientScr_SetSessionState = reinterpret_cast<ClientScr_SetSessionState_t>(0x82366938);
    ClientScr_GetSessionState_t ClientScr_GetSessionState = reinterpret_cast<ClientScr_GetSessionState_t>(0x82366B38);
    ClientScr_SetMaxHealth_t ClientScr_SetMaxHealth = reinterpret_cast<ClientScr_SetMaxHealth_t>(0x823666A8);
    ClientScr_SetScore_t ClientScr_SetScore = reinterpret_cast<ClientScr_SetScore_t>(0x82366670);
    ClientScr_SetStatusIcon_t ClientScr_SetStatusIcon = reinterpret_cast<ClientScr_SetStatusIcon_t>(0x82366E58);
    ClientScr_GetStatusIcon_t ClientScr_GetStatusIcon = reinterpret_cast<ClientScr_GetStatusIcon_t>(0x82366DF8);
    ClientScr_SetHeadIcon_t ClientScr_SetHeadIcon = reinterpret_cast<ClientScr_SetHeadIcon_t>(0x82366D60);
    ClientScr_GetHeadIcon_t ClientScr_GetHeadIcon = reinterpret_cast<ClientScr_GetHeadIcon_t>(0x82366CC8);
    ClientScr_SetHeadIconTeam_t ClientScr_SetHeadIconTeam = reinterpret_cast<ClientScr_SetHeadIconTeam_t>(0x823667E8);
    ClientScr_GetHeadIconTeam_t ClientScr_GetHeadIconTeam = reinterpret_cast<ClientScr_GetHeadIconTeam_t>(0x82366AB0);
    ClientScr_SetSpectatorClient_t ClientScr_SetSpectatorClient = reinterpret_cast<ClientScr_SetSpectatorClient_t>(0x82366610);
    ClientScr_SetKillCamEntity_t ClientScr_SetKillCamEntity = reinterpret_cast<ClientScr_SetKillCamEntity_t>(0x823665B0);
    ClientScr_SetArchiveTime_t ClientScr_SetArchiveTime = reinterpret_cast<ClientScr_SetArchiveTime_t>(0x82366568);
    ClientScr_GetArchiveTime_t ClientScr_GetArchiveTime = reinterpret_cast<ClientScr_GetArchiveTime_t>(0x82366A68);
    ClientScr_SetPSOffsetTime_t ClientScr_SetPSOffsetTime = reinterpret_cast<ClientScr_SetPSOffsetTime_t>(0x82366530);
    ClientScr_GetPSOffsetTime_t ClientScr_GetPSOffsetTime = reinterpret_cast<ClientScr_GetPSOffsetTime_t>(0x82366A60);

    client_fields_s client_fields_extended[] = {
        // Original fields
        {"name", 0, F_LSTRING, ClientScr_ReadOnly, ClientScr_GetName},
        {"sessionteam", 0, F_STRING, ClientScr_SetSessionTeam, ClientScr_GetSessionTeam},
        {"sessionstate", 0, F_STRING, ClientScr_SetSessionState, ClientScr_GetSessionState},
        {"maxhealth", 12284, F_INT, ClientScr_SetMaxHealth, nullptr},
        {"score", 12156, F_INT, ClientScr_SetScore, nullptr},
        {"deaths", 12160, F_INT, nullptr, nullptr},
        {"statusicon", 0, F_STRING, ClientScr_SetStatusIcon, ClientScr_GetStatusIcon},
        {"headicon", 0, F_STRING, ClientScr_SetHeadIcon, ClientScr_GetHeadIcon},
        {"headiconteam", 0, F_STRING, ClientScr_SetHeadIconTeam, ClientScr_GetHeadIconTeam},
        {"kills", 12164, F_INT, nullptr, nullptr},
        {"assists", 12168, F_INT, nullptr, nullptr},
        {"hasradar", 12712, F_INT, nullptr, nullptr},
        {"spectatorclient", 12140, F_INT, ClientScr_SetSpectatorClient, nullptr},
        {"killcamentity", 12144, F_INT, ClientScr_SetKillCamEntity, nullptr},
        {"archivetime", 12152, F_FLOAT, ClientScr_SetArchiveTime, ClientScr_GetArchiveTime},
        {"psoffsettime", 12448, F_INT, ClientScr_SetPSOffsetTime, ClientScr_GetPSOffsetTime},
        {"pers", 12172, F_OBJECT, ClientScr_ReadOnly, nullptr},

        // Custom fields
        {"noclip", 12456, F_INT, nullptr, nullptr},
        {"ufo", 12460, F_INT, nullptr, nullptr},
        {"buttons", 12472, F_INT, nullptr, nullptr},

        {nullptr, 0, F_INT, nullptr, nullptr}};

    auto client_fields = reinterpret_cast<client_fields_s *>(0x82047A18);
    auto entity_fields = reinterpret_cast<ent_field_t *>(0x82046E00);

    GScr_AddFieldsForClient_t GScr_AddFieldsForClient = reinterpret_cast<GScr_AddFieldsForClient_t>(0x82366780);
    Scr_AddClassField_t Scr_AddClassField = reinterpret_cast<Scr_AddClassField_t>(0x8221B2A0);

    Scr_SetEntityField_t Scr_SetEntityField = reinterpret_cast<Scr_SetEntityField_t>(0x8224FA28);
    Scr_GetEntityField_t Scr_GetEntityField = reinterpret_cast<Scr_GetEntityField_t>(0x8224F5C8);

    Scr_SetGenericField_t Scr_SetGenericField = reinterpret_cast<Scr_SetGenericField_t>(0x8224F6B0);
    Scr_GetGenericField_t Scr_GetGenericField = reinterpret_cast<Scr_GetGenericField_t>(0x8224F1A0);

    Detour GScr_AddFieldsForClient_Detour;

    void GScr_AddFieldsForClient_Hook()
    {
        auto client_field = client_fields_extended;
        while (client_field->name != nullptr)
        {
            int field_index = client_field - client_fields_extended;
            Scr_AddClassField(0, client_field->name, (unsigned short)(field_index) | CLIENT_FIELD_MASK);
            client_field++;
        }
    }

    Detour Scr_SetEntityField_Detour;

    int Scr_SetEntityField_Hook(int entnum, int offset)
    {
        xbox::DbgPrint("Scr_SetEntityField entnum=%d offset=%d\n", entnum, offset);

        auto entity = &g_entities[entnum];

        // Client field case
        if ((offset & CLIENT_FIELD_MASK) == CLIENT_FIELD_MASK)
        {
            if (!entity->client)
            {
                return 0;
            }

            // Using our extended client fields
            const client_fields_s *field = &client_fields_extended[offset & 0xFFFF3FFF];

            // Check if there's a setter function
            if (field->setter)
            {
                field->setter(entity->client, field);
                return 1;
            }

            Scr_SetGenericField((unsigned char *)entity->client, field->type, field->ofs);
            return 1;
        }
        else
        {
            // Entity field case
            const ent_field_t *field = &entity_fields[offset];

            if (field->callback)
            {
                field->callback(entity, field->ofs);
                return 1;
            }

            Scr_SetGenericField(reinterpret_cast<unsigned __int8 *>(entity), field->type, field->ofs);
            return 1;
        }
    }

    Detour Scr_GetEntityField_Detour;

    void Scr_GetEntityField_Hook(int entnum, int offset)
    {
        auto entity = &g_entities[entnum];

        if ((offset & CLIENT_FIELD_MASK) == CLIENT_FIELD_MASK)
        {
            if (!entity->client)
            {
                return;
            }

            const client_fields_s *field = &client_fields_extended[offset & 0xFFFF3FFF];

            if (field->getter)
            {
                field->getter(entity->client, field);
                return;
            }

            Scr_GetGenericField(reinterpret_cast<unsigned __int8 *>(entity->client), field->type, field->ofs);
        }
        else
        {
            const ent_field_t *field = &entity_fields[offset];
            Scr_GetGenericField(reinterpret_cast<unsigned __int8 *>(entity), field->type, field->ofs);
        }
    }

    void init_gscr_fields()
    {
        GScr_AddFieldsForClient_Detour = Detour(GScr_AddFieldsForClient, GScr_AddFieldsForClient_Hook);
        GScr_AddFieldsForClient_Detour.Install();

        Scr_GetEntityField_Detour = Detour(Scr_GetEntityField, Scr_GetEntityField_Hook);
        Scr_GetEntityField_Detour.Install();

        Scr_SetEntityField_Detour = Detour(Scr_SetEntityField, Scr_SetEntityField_Hook);
        Scr_SetEntityField_Detour.Install();
    }
}
