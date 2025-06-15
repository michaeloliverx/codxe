#pragma once

#include "structs.h"

namespace t4
{
    namespace mp
    {
        // Variables
        static const int BUILTIN_FUNCTION_COUNT = 266;
        static auto builtinFunctions = reinterpret_cast<BuiltinFunctionDef *>(0x82485C60);
        static auto client_fields = reinterpret_cast<client_fields_s *>(0x82026B80);
        static auto cm = reinterpret_cast<clipMap_t *>(0x82DD4F80);
        static auto g_entities = reinterpret_cast<gentity_s *>(0x82BAD1B0);
        static auto level = reinterpret_cast<level_locals_t *>(0x82D6A440);

        // Functions
        static auto CG_BoldGameMessage = reinterpret_cast<void (*)(int localClientNum, const char *msg)>(0x8216EC88);
        static auto CG_DrawActive = reinterpret_cast<void (*)(int localClientNum)>(0x82159560);
        static auto CG_GameMessage = reinterpret_cast<void (*)(int localClientNum, const char *msg)>(0x8216EC68);

        static auto CM_LoadMap = reinterpret_cast<void (*)(const char *name)>(0x821A4CB0);

        static auto DB_FindXAssetHeader = reinterpret_cast<XAssetHeader *(*)(const XAssetType type, const char *name, bool errorIfMissing, int waitTime)>(0x821EA5D8);
        static auto DB_GetAllXAssetOfType_FastFile = reinterpret_cast<int (*)(XAssetType type, XAssetHeader *assets, int maxCount)>(0x821E7428);

        static auto Dvar_FindMalleableVar = reinterpret_cast<dvar_s *(*)(const char *dvarName)>(0x822BDFE8);
        static auto Dvar_GetString = reinterpret_cast<const char *(*)(const char *dvarName)>(0x822BE230);
        static auto Dvar_RegisterString = reinterpret_cast<dvar_s *(*)(const char *dvarName, const char *value, unsigned __int16 flags, const char *description)>(0x822BF370);

        static auto GScr_AddFieldsForClient = reinterpret_cast<void (*)()>(0x8220A270);

        static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x822B9CC0);

        static auto Player_GetMethod = reinterpret_cast<BuiltinPlayerMethod (*)(const char **pName)>(0x82211398);

        static auto R_AddCmdDrawText = reinterpret_cast<void (*)(const char *text, int fontSize, Font_s *font, float x, float y, float scaleX, float scaleY, float rotation, const float *color, int flags)>(0x82401C30);
        static auto R_CheckDvarModified = reinterpret_cast<int (*)(const dvar_s *dvar)>(0x8240D860);

        static auto Scr_AddArray = reinterpret_cast<void (*)(scriptInstance_t inst)>(0x82345C80);
        static auto Scr_AddClassField = reinterpret_cast<void (*)(unsigned int classnum, const char *name, unsigned __int16 offset, scriptInstance_t inst)>(0x823414F0);
        static auto Scr_AddInt = reinterpret_cast<void (*)(int value, scriptInstance_t inst)>(0x82345668);
        static auto Scr_AddSourceBuffer = reinterpret_cast<char *(*)(scriptInstance_t inst, const char *filename, const char *extFilename, const char *codePos, bool archive)>(0x82339EF8);
        static auto Scr_Error = reinterpret_cast<void (*)(const char *error, scriptInstance_t inst)>(0x8234BE08);
        static auto Scr_GetFloat = reinterpret_cast<float (*)(unsigned int index, scriptInstance_t inst)>(0x8234B250);
        static auto Scr_GetFunction = reinterpret_cast<BuiltinFunction (*)(const char **pName, int *type)>(0x822416B0);
        static auto Scr_GetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs, scriptInstance_t inst, unsigned int whichbits)>(0x82255100);
        static auto Scr_GetInt = reinterpret_cast<int (*)(unsigned int index, scriptInstance_t inst, __int64 a3, __int64 a4)>(0x8234AFD0);
        static auto Scr_GetNumParam = reinterpret_cast<unsigned int (*)(scriptInstance_t inst)>(0x82345650);
        static auto Scr_GetObjectField = reinterpret_cast<void (*)(unsigned int classnum, int entnum, int offset, scriptInstance_t inst)>(0x822556B0);
        static auto Scr_GetString = reinterpret_cast<const char *(*)(unsigned int index, scriptInstance_t inst)>(0x8234B550);
        static auto Scr_GetVector = reinterpret_cast<void (*)(unsigned int index, float *vectorValue, scriptInstance_t inst, __int64 a4)>(0x8234B790);
        static auto Scr_MakeArray = reinterpret_cast<void (*)(scriptInstance_t inst)>(0x82345BF8);
        static auto Scr_SetClientField = reinterpret_cast<void (*)(gclient_s *client, int offset, scriptInstance_t inst)>(0x8220A2D0);
        static auto Scr_SetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs, scriptInstance_t inst)>(0x82254E90);

        static auto SV_GameSendServerCommand = reinterpret_cast<void (*)(int clientNum, svscmd_type type, const char *text)>(0x82285FA8);

        static auto UI_DrawBuildNumber = reinterpret_cast<void (*)()>(0x8229BCF8);

        static auto va = reinterpret_cast<char *(*)(char *format, ...)>(0x822C38D8);
    }
}
