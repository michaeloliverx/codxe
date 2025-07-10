#pragma once

#include "structs.h"

namespace t4
{
    namespace mp
    {
        // Variables
        static const int BUILTIN_FUNCTION_COUNT = 266;
        static auto builtinFunctions = reinterpret_cast<BuiltinFunctionDef *>(0x82485C60);
        static const int BUILTIN_PLAYER_METHOD_COUNT = 92;
        static auto builtin_player_methods = reinterpret_cast<BuiltinFunctionDef *>(0x820275A0);
        static auto client_fields = reinterpret_cast<client_fields_s *>(0x82026B80); // null-terminated array
        static auto cm = reinterpret_cast<clipMap_t *>(0x82DD4F80);
        static auto ent_fields = reinterpret_cast<ent_field_t *>(0x82035BF0); // null-terminated array
        static auto g_entities = reinterpret_cast<gentity_s *>(0x82BAD1B0);
        static auto level = reinterpret_cast<level_locals_t *>(0x82D6A440);
        static auto svsHeader = reinterpret_cast<serverStaticHeader_t *>(0x84f85100);
        static auto scrVarPub = reinterpret_cast<scrVarPub_t *>(0x85AC2F00);
        static auto varclipMap_t = reinterpret_cast<clipMap_t **>(0x82756700);

        const int NUM_BSP_ONLY_SPAWNS = 14;
        static auto s_bspOnlySpawns = reinterpret_cast<SpawnFuncEntry *>(0x82035CC0);

        const int NUM_BSP_OR_DYNAMIC_SPAWNS = 6;
        static auto s_bspOrDynamicSpawns = reinterpret_cast<SpawnFuncEntry *>(0x82035D30);

        // Functions
        static auto BG_CalculateView_IdleAngles = reinterpret_cast<void (*)(viewState_t *vs, float *angles)>(0x82147018);
        static auto BG_CalculateWeaponPosition_IdleAngles = reinterpret_cast<void (*)(weaponState_t *ws, float *angles)>(0x82147AC0);

        static auto CG_BoldGameMessage = reinterpret_cast<void (*)(int localClientNum, const char *msg)>(0x8216EC88);
        static auto CG_DrawActive = reinterpret_cast<void (*)(int localClientNum)>(0x82159560);
        static auto CG_GameMessage = reinterpret_cast<void (*)(int localClientNum, const char *msg)>(0x8216EC68);
        static auto CG_Init = reinterpret_cast<void (*)(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum)>(0x82171A30);

        static auto CM_EntityString = reinterpret_cast<const char *(*)()>(0x82264788);
        static auto CM_LoadMap = reinterpret_cast<void (*)(const char *name)>(0x821A4CB0);

        static auto Cbuf_ExecuteBuffer = reinterpret_cast<void (*)(int localClientNum, int controllerIndex, const char *buffer)>(0x822711A8);

        static auto CL_WritePacket = reinterpret_cast<void (*)(int localClientNum)>(0x821B0F30);

        static auto DB_FindXAssetHeader = reinterpret_cast<XAssetHeader *(*)(const XAssetType type, const char *name, bool errorIfMissing, int waitTime)>(0x821EA5D8);
        static auto DB_GetAllXAssetOfType_FastFile = reinterpret_cast<int (*)(XAssetType type, XAssetHeader *assets, int maxCount)>(0x821E7428);

        static auto Dvar_FindMalleableVar = reinterpret_cast<dvar_s *(*)(const char *dvarName)>(0x822BDFE8);
        static auto Dvar_GetBool = reinterpret_cast<dvar_s *(*)(const char *dvarName)>(0x822BE0E8);
        static auto Dvar_GetString = reinterpret_cast<const char *(*)(const char *dvarName)>(0x822BE230);
        static auto Dvar_RegisterBool = reinterpret_cast<dvar_s *(*)(const char *dvarName, bool value, DvarFlags flags, const char *description)>(0x822BF198);
        static auto Dvar_RegisterString = reinterpret_cast<dvar_s *(*)(const char *dvarName, const char *value, DvarFlags flags, const char *description)>(0x822BF370);
        static auto Dvar_SetBoolByName = reinterpret_cast<void (*)(const char *dvarName, bool value)>(0x822BFB20);

        static auto GScr_AddFieldsForClient = reinterpret_cast<void (*)()>(0x8220A270);
        static auto GScr_AllocString = reinterpret_cast<int (*)(const char *s)>(0x8222D368);

        static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x822B9CC0);

        static auto Load_clipMap_t = reinterpret_cast<void (*)(bool atStreamStart)>(0x821E4220);

        static auto Menu_IsMenuOpenAndVisible = reinterpret_cast<bool (*)(int localClientNum, const char *menuName)>(0x8229F6C0);
        static auto Menus_OpenByName = reinterpret_cast<void (*)(UiContext *dc, const char *menuName)>(0x822AFE88);

        static auto Pmove = reinterpret_cast<void (*)(pmove_t *pm)>(0x8213FAE8);
        static auto PmoveSingle = reinterpret_cast<void (*)(pmove_t *pm)>(0x8213ECE8);

        static auto Player_GetMethod = reinterpret_cast<BuiltinMethod (*)(const char **pName)>(0x82211398);

        static auto R_AddCmdDrawText = reinterpret_cast<void (*)(const char *text, int fontSize, Font_s *font, float x, float y, float scaleX, float scaleY, float rotation, const float *color, int flags)>(0x82401C30);
        static auto R_CheckDvarModified = reinterpret_cast<int (*)(const dvar_s *dvar)>(0x8240D860);

        static auto ScriptEnt_GetMethod = reinterpret_cast<BuiltinMethod (*)(const char **pName)>(0x82244B50);
        static auto Scr_AddArray = reinterpret_cast<void (*)(scriptInstance_t inst)>(0x82345C80);
        static auto Scr_AddClassField = reinterpret_cast<void (*)(unsigned int classnum, const char *name, unsigned __int16 offset, scriptInstance_t inst)>(0x823414F0);
        static auto Scr_AddInt = reinterpret_cast<void (*)(int value, scriptInstance_t inst)>(0x82345668);
        static auto Scr_AddSourceBuffer = reinterpret_cast<char *(*)(scriptInstance_t inst, const char *filename, const char *extFilename, const char *codePos, bool archive)>(0x82339EF8);
        static auto Scr_GetEntity = reinterpret_cast<gentity_s *(*)(unsigned int index)>(0x82254018);
        static auto Scr_Error = reinterpret_cast<void (*)(const char *error, scriptInstance_t inst)>(0x8234BE08);
        static auto Scr_ErrorInternal = reinterpret_cast<void (*)()>(0x82343F78);
        static auto Scr_GetFloat = reinterpret_cast<float (*)(unsigned int index, scriptInstance_t inst)>(0x8234B250);
        static auto Scr_GetFunction = reinterpret_cast<BuiltinFunction (*)(const char **pName, int *type)>(0x822416B0);
        static auto Scr_GetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs, scriptInstance_t inst, unsigned int whichbits)>(0x82255100);
        static auto Scr_GetInt = reinterpret_cast<int (*)(unsigned int index, scriptInstance_t inst, __int64 a3, __int64 a4)>(0x8234AFD0);
        static auto Scr_GetNumParam = reinterpret_cast<unsigned int (*)(scriptInstance_t inst)>(0x82345650);
        static auto Scr_GetObjectField = reinterpret_cast<void (*)(unsigned int classnum, int entnum, int offset, scriptInstance_t inst)>(0x822556B0);
        static auto Scr_GetString = reinterpret_cast<const char *(*)(unsigned int index, scriptInstance_t inst)>(0x8234B550);
        static auto Scr_GetVector = reinterpret_cast<void (*)(unsigned int index, float *vectorValue, scriptInstance_t inst, __int64 a4)>(0x8234B790);
        static auto Scr_MakeArray = reinterpret_cast<void (*)(scriptInstance_t inst)>(0x82345BF8);
        static auto Scr_GetMethod = reinterpret_cast<BuiltinMethod (*)(const char **pName, int *type)>(0x82241760);
        static auto Scr_ObjectError = reinterpret_cast<void (*)(const char *error, scriptInstance_t inst)>(0x82345EF0);
        static auto Scr_ParamError = reinterpret_cast<void (*)(unsigned int index, const char *error, scriptInstance_t inst)>(0x82345E70);
        static auto Scr_SetClientField = reinterpret_cast<void (*)(gclient_s *client, int offset, scriptInstance_t inst)>(0x8220A2D0);
        static auto Scr_SetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs, scriptInstance_t inst)>(0x82254E90);

        static auto String_Parse = reinterpret_cast<int (*)(const char **p, char *out, int len)>(0x822A8680);

        static auto SV_ClientThink = reinterpret_cast<void (*)(client_t *cl, usercmd_s *cmd)>(0x82284D50);
        static auto SV_GameSendServerCommand = reinterpret_cast<void (*)(int clientNum, svscmd_type type, const char *text)>(0x82285FA8);
        static auto SV_LinkEntity = reinterpret_cast<void (*)(gentity_s *ent)>(0x82290C38);
        static auto SV_SetBrushModel = reinterpret_cast<int (*)(gentity_s *ent)>(0x82286190);
        static auto SV_UnlinkEntity = reinterpret_cast<void (*)(gentity_s *ent)>(0x82290BF0);
        static auto Sys_SnapVector = reinterpret_cast<void (*)(float *v)>(0x822EA5A0);

        static auto UI_DrawBuildNumber = reinterpret_cast<void (*)()>(0x8229BCF8);
        static auto UI_RunMenuScript = reinterpret_cast<void (*)(int localClientNum, const char **args, const char *actualScript)>(0x822A29B8);

        static auto va = reinterpret_cast<char *(*)(char *format, ...)>(0x822C38D8);
    }
}
