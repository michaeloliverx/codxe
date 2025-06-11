#include "gsc_client_fields.h"
#include "common.h"

const unsigned int CLIENT_FIELD_MASK = 0xC000;

namespace t4
{
    namespace mp
    {
        client_fields_s client_fields_extended[] = {
            // Original fields
            {"name", 0, F_LSTRING, 0x00000000, reinterpret_cast<ClientFieldSetter>(0x82209910), reinterpret_cast<ClientFieldGetter>(0x82209A48)},
            {"sessionteam", 0, F_STRING, 0x00000000, reinterpret_cast<ClientFieldSetter>(0x82209948), reinterpret_cast<ClientFieldGetter>(0x82209AB8)},
            {"maxhealth", 14936, F_INT, 0x00000000, reinterpret_cast<ClientFieldSetter>(0x82209CD8), nullptr},
            {"headiconteam", 0, F_STRING, 0x00000000, reinterpret_cast<ClientFieldSetter>(0x82209FE8), reinterpret_cast<ClientFieldGetter>(0x8220A110)},
            {"hasradar", 15444, F_INT, 0x00000000, nullptr, nullptr},
            {"deaths", 14788, F_INT, 0x00000000, nullptr, nullptr},
            {"disallowvehicleusage", 15456, F_INT, 0x00000000, nullptr, nullptr},
            {"killcamentity", 14772, F_INT, 0x00000000, reinterpret_cast<ClientFieldSetter>(0x82209E10), nullptr},
            {"kills", 14792, F_INT, 0x00000000, nullptr, nullptr},
            {"assists", 14796, F_INT, 0x00000000, nullptr, nullptr},
            {"score", 14784, F_INT, 0x00000000, reinterpret_cast<ClientFieldSetter>(0x82209D68), nullptr},
            {"sessionstate", 0, F_STRING, 0x00000000, reinterpret_cast<ClientFieldSetter>(0x82209B40), reinterpret_cast<ClientFieldGetter>(0x82209C50)},
            {"statusicon", 0, F_STRING, 0x00000000, reinterpret_cast<ClientFieldSetter>(0x82209E78), reinterpret_cast<ClientFieldGetter>(0x82209EB8)},
            {"spectatorclient", 14768, F_INT, 0x00000000, reinterpret_cast<ClientFieldSetter>(0x82209DA8), nullptr},
            {"archivetime", 14780, F_FLOAT, 0x00000000, reinterpret_cast<ClientFieldSetter>(0x8220A190), reinterpret_cast<ClientFieldGetter>(0x8220A1E0)},
            {"psoffsettime", 15124, F_INT, 0x00000000, reinterpret_cast<ClientFieldSetter>(0x8220A228), reinterpret_cast<ClientFieldGetter>(0x8220A260)},
            {"pers", 14800, F_OBJECT, 0x00000000, reinterpret_cast<ClientFieldSetter>(0x82209910), nullptr},
            {"usingturret", 204, F_BITFLAG, 0x00000001, reinterpret_cast<ClientFieldSetter>(0x82209910), nullptr},
            {"usingvehicle", 204, F_BITFLAG, 0x00000002, reinterpret_cast<ClientFieldSetter>(0x82209910), nullptr},
            {"vehicleposition", 3820, F_INT, 0x00000000, reinterpret_cast<ClientFieldSetter>(0x82209910), nullptr},
            {"headicon", 0, F_STRING, 0x00000000, reinterpret_cast<ClientFieldSetter>(0x82209F08), reinterpret_cast<ClientFieldGetter>(0x82209F68)},

            // Custom fields
            {"noclip", offsetof(gclient_s, noclip), F_INT, 0x00000000, nullptr, nullptr},
            {"ufo", offsetof(gclient_s, ufo), F_INT, 0x00000000, nullptr, nullptr},

            // Terminator
            {nullptr, 0, F_INT, 0x00000000, nullptr, nullptr}};

        Detour GScr_AddFieldsForClient_Detour;

        void GScr_AddFieldsForClient_Hook()
        {
            auto field = client_fields_extended;
            while (field->name != nullptr)
            {
                int field_index = field - client_fields_extended;
                Scr_AddClassField(0, field->name, (unsigned short)(field_index) | CLIENT_FIELD_MASK, SCRIPTINSTANCE_SERVER);
                field++;
            }
        }

        Detour Scr_SetClientField_Detour;

        void Scr_SetClientField_Hook(gclient_s *client, unsigned int offset, scriptInstance_t inst)
        {
            auto field = &client_fields_extended[offset];
            if (field->setter)
                field->setter(client, field);
            else
                Scr_SetGenericField((unsigned char *)client, field->type, field->ofs, inst);
        }

        Detour Scr_GetObjectField_Detour;

        void Scr_GetObjectField_Hook(unsigned int classnum, int entnum, int offset, scriptInstance_t inst)
        {
            auto ent = &g_entities[entnum];
            // Check if the offset is a client field
            if (classnum == 0 && (offset & CLIENT_FIELD_MASK) == CLIENT_FIELD_MASK)
            {
                auto field = &client_fields_extended[offset & ~CLIENT_FIELD_MASK];
                if (field->getter)
                    field->getter(ent->client, field);
                else
                    // If no getter is defined, use the generic field getter
                    Scr_GetGenericField((unsigned char *)ent->client, field->type, field->ofs, inst, field->whichbits);
                return;
            }
            // Otherwise, call the original function
            else
                Scr_GetObjectField_Detour.GetOriginal<decltype(Scr_GetObjectField)>()(classnum, entnum, offset, inst);
        }

        GSCClientFields::GSCClientFields()
        {
            GScr_AddFieldsForClient_Detour = Detour(GScr_AddFieldsForClient, GScr_AddFieldsForClient_Hook);
            GScr_AddFieldsForClient_Detour.Install();

            Scr_SetClientField_Detour = Detour(Scr_SetClientField, Scr_SetClientField_Hook);
            Scr_SetClientField_Detour.Install();

            Scr_GetObjectField_Detour = Detour(Scr_GetObjectField, Scr_GetObjectField_Hook);
            Scr_GetObjectField_Detour.Install();
        }

        GSCClientFields::~GSCClientFields()
        {

            GScr_AddFieldsForClient_Detour.Remove();
            Scr_SetClientField_Detour.Remove();
            Scr_GetObjectField_Detour.Remove();
        }
    }
}
