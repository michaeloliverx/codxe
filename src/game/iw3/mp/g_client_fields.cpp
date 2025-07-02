#include "common.h"
#include "g_client_fields.h"

const unsigned int CLIENT_FIELD_MASK = 0xC000;

namespace iw3
{
    namespace mp
    {
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
}
