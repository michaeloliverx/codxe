#pragma once

#include "structs.h"

namespace iw4
{
    namespace sp
    {
        // Functions
        static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x822DB330);

        static auto Scr_AddSourceBuffer = reinterpret_cast<char *(*)(const char *filename, const char *extFilename)>(0x8229E2D0);

        static auto GScr_AddFieldsForEntity = reinterpret_cast<void (*)()>(0x82229B08);

        static auto Scr_AddClassField = reinterpret_cast<void (*)(unsigned int classnum, const char *name, unsigned __int16 offset)>(0x822A8AF0);
        static auto GScr_AddFieldsForClient = reinterpret_cast<void (*)()>(0x821E7760);
        static auto Scr_SetClientField = reinterpret_cast<void (*)(gclient_s *client, int offset)>(0x821E77B8);
        static auto Scr_SetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs)>(0x8222AAA8);
        static auto Scr_GetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs)>(0x8222AC78);
        static auto Scr_GetObjectField = reinterpret_cast<void (*)(unsigned int classnum, int entnum, int offset)>(0x8222B228);

        static auto ClientScr_ReadOnly = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821E7730);
        static auto ClientScr_GetName = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821E7720);

        static auto Scr_AddString = reinterpret_cast<void (*)(const char *value)>(0x822ACF48);
        static auto Scr_GetString = reinterpret_cast<const char *(*)(unsigned int index)>(0x822B230);
        static auto Scr_AddInt = reinterpret_cast<void (*)(int value)>(0x822ACC70);
        static auto Scr_GetInt = reinterpret_cast<int (*)(unsigned int index)>(0x822B1CC8);
        static auto va = reinterpret_cast<char *(*)(const char *format, ...)>(0x822E8568);
        static auto Scr_Error = reinterpret_cast<void (*)(const char *error)>(0x822AD3C8);
        static auto Scr_ParamError = reinterpret_cast<void (*)(unsigned int index, const char *error)>(0x822AD558);
        static auto Scr_ObjectError = reinterpret_cast<void (*)(const char *error)>(0x822AD5C0);

        static auto Player_GetMethod = reinterpret_cast<BuiltinMethod (*)(const char **pName)>(0x821EEC20);

        // Data
        static auto fields = reinterpret_cast<client_fields_s *>(0x8201ACF0);
        static auto g_clients = reinterpret_cast<gclient_s *>(0x82EAC5E8);
        static auto g_entities = reinterpret_cast<gentity_s *>(0x82CC67B8);
        static auto level = reinterpret_cast<level_locals_t *>(0x82EC2598);
        static auto methods_1 = reinterpret_cast<BuiltinMethodDef *>(0x8201CA80);
        static const int METHODS_1_COUNT = 35;
    }
}
