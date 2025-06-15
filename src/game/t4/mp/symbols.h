#pragma once

// #include "common.h"
#include "structs.h"

namespace t4
{
    namespace mp
    {
        // Variables
        extern clipMap_t *cm;
        extern client_fields_s *client_fields;
        extern gentity_s *g_entities;

        static const int BUILTIN_FUNCTION_COUNT = 266;
        static auto builtinFunctions = reinterpret_cast<BuiltinFunctionDef *>(0x82485C60);

        // Functions

        static auto CG_DrawActive = reinterpret_cast<void (*)(int localClientNum)>(0x82159560);
        static auto CG_GameMessage = reinterpret_cast<void (*)(int localClientNum, const char *msg)>(0x8216EC68);

        static auto CM_LoadMap = reinterpret_cast<void (*)(const char *name)>(0x821A4CB0);

        static auto R_CheckDvarModified = reinterpret_cast<int (*)(const dvar_s *dvar)>(0x8240D860);
        static auto Dvar_FindMalleableVar = reinterpret_cast<dvar_s *(*)(const char *dvarName)>(0x822BDFE8);
        static auto Dvar_GetString = reinterpret_cast<const char *(*)(const char *dvarName)>(0x822BE230);
        static auto Dvar_RegisterString = reinterpret_cast<dvar_s *(*)(const char *dvarName, const char *value, unsigned __int16 flags, const char *description)>(0x822BF370);

        // typedef void (*CG_BoldGameMessage_t)(int localClientNum, const char *msg);
        // CG_BoldGameMessage_t CG_BoldGameMessage = reinterpret_cast<CG_BoldGameMessage_t>(0x8216EC88);

        typedef void (*Scr_AddClassField_t)(unsigned int classnum, const char *name, unsigned __int16 offset, scriptInstance_t inst);
        extern Scr_AddClassField_t Scr_AddClassField;

        typedef void (*Scr_SetClientField_t)(gclient_s *client, int offset, scriptInstance_t inst);
        extern Scr_SetClientField_t Scr_SetClientField;

        typedef void (*Scr_SetGenericField_t)(unsigned __int8 *b, fieldtype_t type, int ofs, scriptInstance_t inst);
        extern Scr_SetGenericField_t Scr_SetGenericField;

        typedef void (*Scr_GetObjectField_t)(unsigned int classnum, int entnum, int offset, scriptInstance_t inst);
        extern Scr_GetObjectField_t Scr_GetObjectField;

        typedef void (*Scr_GetGenericField_t)(unsigned __int8 *b, fieldtype_t type, int ofs, scriptInstance_t inst, unsigned int whichbits);
        extern Scr_GetGenericField_t Scr_GetGenericField;

        typedef void (*GScr_AddFieldsForClient_t)();
        extern GScr_AddFieldsForClient_t GScr_AddFieldsForClient;

        typedef BuiltinFunction (*Scr_GetFunction_t)(const char **pName, int *type);
        extern Scr_GetFunction_t Scr_GetFunction;

        // typedef BuiltinPlayerMethod (*Player_GetMethod_t)(const char **pName);
        // Player_GetMethod_t Player_GetMethod = reinterpret_cast<Player_GetMethod_t>(0x82211398);

        static auto Scr_GetVector = reinterpret_cast<void (*)(unsigned int index, float *vectorValue, scriptInstance_t inst, __int64 a4)>(0x8234B790);

        // typedef int (*Scr_GetInt_t)(unsigned int a1, int a2, __int64 a3, __int64 a4);
        // Scr_GetInt_t Scr_GetInt = reinterpret_cast<Scr_GetInt_t>(0x8234AFD0);

        static auto Scr_AddInt = reinterpret_cast<void (*)(int value, scriptInstance_t inst)>(0x82345668);

        static auto Scr_MakeArray = reinterpret_cast<void (*)(scriptInstance_t inst)>(0x82345BF8);
        static auto Scr_AddArray = reinterpret_cast<void (*)(scriptInstance_t inst)>(0x82345C80);

        // typedef void (*SV_GameSendServerCommand_t)(int clientNum, svscmd_type type, const char *text);
        // SV_GameSendServerCommand_t SV_GameSendServerCommand = reinterpret_cast<SV_GameSendServerCommand_t>(0x82285FA8);

        typedef XAssetHeader *(*DB_FindXAssetHeader_t)(const XAssetType type, const char *name, bool errorIfMissing, int waitTime);
        extern DB_FindXAssetHeader_t DB_FindXAssetHeader;

        typedef int (*DB_GetAllXAssetOfType_FastFile_t)(XAssetType type, XAssetHeader *assets, int maxCount);
        extern DB_GetAllXAssetOfType_FastFile_t DB_GetAllXAssetOfType_FastFile;

        typedef void *(*Hunk_AllocateTempMemoryHighInternal_t)(int size);
        extern Hunk_AllocateTempMemoryHighInternal_t Hunk_AllocateTempMemoryHighInternal;

        typedef void (*R_AddCmdDrawText_t)(char const *, int, Font_s *, float, float, float, float, float, float const *const, int);
        extern R_AddCmdDrawText_t R_AddCmdDrawText;

        typedef char *(*Scr_AddSourceBuffer_t)(scriptInstance_t a1, const char *filename, const char *extFilename, const char *codePos, bool archive);
        extern Scr_AddSourceBuffer_t Scr_AddSourceBuffer;

        typedef void (*UI_DrawBuildNumber_t)(const int localClientNum);
        extern UI_DrawBuildNumber_t UI_DrawBuildNumber;

        static auto va = reinterpret_cast<char *(*)(char *format, ...)>(0x822C38D8);
    }
}
