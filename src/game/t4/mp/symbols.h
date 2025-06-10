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

        // // Functions
        // typedef void (*CG_GameMessage_t)(int localClientNum, const char *msg);
        // CG_GameMessage_t CG_GameMessage = reinterpret_cast<CG_GameMessage_t>(0x8216EC68);

        // typedef void (*CG_BoldGameMessage_t)(int localClientNum, const char *msg);
        // CG_BoldGameMessage_t CG_BoldGameMessage = reinterpret_cast<CG_BoldGameMessage_t>(0x8216EC88);

        // typedef void (*Scr_AddClassField_t)(unsigned int classnum, const char *name, unsigned __int16 offset, int a4);
        // Scr_AddClassField_t Scr_AddClassField = reinterpret_cast<Scr_AddClassField_t>(0x823414F0);

        // typedef void (*Scr_SetClientField_t)(gclient_s *client, int offset, int a3);
        // Scr_SetClientField_t Scr_SetClientField = reinterpret_cast<Scr_SetClientField_t>(0x8220A2D0);

        // typedef void (*Scr_GetObjectField_t)(unsigned int classnum, int entnum, int offset, int a4);
        // Scr_GetObjectField_t Scr_GetObjectField = reinterpret_cast<Scr_GetObjectField_t>(0x822556B0);

        // typedef void (*GScr_AddFieldsForClient_t)();
        // GScr_AddFieldsForClient_t GScr_AddFieldsForClient = reinterpret_cast<GScr_AddFieldsForClient_t>(0x8220A270);

        // typedef char *(*Scr_ReadFile_FastFile_t)(const char *filename, const char *extFilename, const char *codePos, bool archive);
        // Scr_ReadFile_FastFile_t Scr_ReadFile_FastFile = reinterpret_cast<Scr_ReadFile_FastFile_t>(0x82339EF8);

        // typedef void (*BuiltinFunction)();
        // typedef void (*BuiltinPlayerMethod)(scr_entref_t entref);

        // typedef BuiltinFunction (*Scr_GetFunction_t)(const char **pName, int *type);
        // Scr_GetFunction_t Scr_GetFunction = reinterpret_cast<Scr_GetFunction_t>(0x822416B0);

        // typedef BuiltinPlayerMethod (*Player_GetMethod_t)(const char **pName);
        // Player_GetMethod_t Player_GetMethod = reinterpret_cast<Player_GetMethod_t>(0x82211398);

        // typedef void (*Scr_GetVector_t)(unsigned int result, float *a2, __int64 a3, __int64 a4);
        // Scr_GetVector_t Scr_GetVector = reinterpret_cast<Scr_GetVector_t>(0x8234B790);

        // typedef int (*Scr_GetInt_t)(unsigned int a1, int a2, __int64 a3, __int64 a4);
        // Scr_GetInt_t Scr_GetInt = reinterpret_cast<Scr_GetInt_t>(0x8234AFD0);

        // typedef void (*Scr_AddInt_t)(int value, int a2);
        // Scr_AddInt_t Scr_AddInt = reinterpret_cast<Scr_AddInt_t>(0x82345668);

        // typedef void (*SV_GameSendServerCommand_t)(int clientNum, svscmd_type type, const char *text);
        // SV_GameSendServerCommand_t SV_GameSendServerCommand = reinterpret_cast<SV_GameSendServerCommand_t>(0x82285FA8);

        typedef XAssetHeader *(*DB_FindXAssetHeader_t)(const XAssetType type, const char *name, bool errorIfMissing, int waitTime);
        extern DB_FindXAssetHeader_t DB_FindXAssetHeader;

        typedef int (*DB_GetAllXAssetOfType_FastFile_t)(XAssetType type, XAssetHeader *assets, int maxCount);
        extern DB_GetAllXAssetOfType_FastFile_t DB_GetAllXAssetOfType_FastFile;

        typedef void (*R_AddCmdDrawText_t)(char const *, int, Font_s *, float, float, float, float, float, float const *const, int);
        extern R_AddCmdDrawText_t R_AddCmdDrawText;

        typedef void (*UI_DrawBuildNumber_t)(const int localClientNum);
        extern UI_DrawBuildNumber_t UI_DrawBuildNumber;

    }
}
