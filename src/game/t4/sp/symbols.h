#pragma once

#include "structs.h"

namespace t4
{
    namespace sp
    {
        // Data
        static auto cm = reinterpret_cast<clipMap_t *>(0x831A0F00);
        static auto g_clients = reinterpret_cast<gclient_s *>(0x82ABAC00);
        static auto g_entities = reinterpret_cast<gentity_s *>(0x8293A280);
        static auto level = reinterpret_cast<level_locals_t *>(0x82AC3930);

        const int BSP_ONLY_SPAWNS_COUNT = 16;
        static auto s_bspOnlySpawns = reinterpret_cast<SpawnFuncEntry *>(0x8202CA40);

        const int BSP_OR_DYNAMIC_SPAWNS_COUNT = 11;
        static auto s_bspOrDynamicSpawns = reinterpret_cast<SpawnFuncEntry *>(0x8202CAC0);

        // Null-terminated array of client fields
        static auto client_fields = reinterpret_cast<client_fields_s *>(0x82019DD0);

        const int ENTITY_TYPE_COUNT = 19;
        static auto entityTypeNames = reinterpret_cast<const char **>(0x824B60C8);

        static auto varclipMap_t = reinterpret_cast<clipMap_t **>(0x8253F2F0);

        // Functions
        static auto BG_AddPredictableEventToPlayerstate = reinterpret_cast<void (*)(unsigned __int8 newEvent, unsigned __int8 eventParm, struct playerState_s *ps)>(0x820EFC08);

        static auto CM_EntityString = reinterpret_cast<const char *(*)()>(0x82241C88);
        static auto CL_CmdButtons = reinterpret_cast<void (*)(int a1, usercmd_s *a2)>(0x822E38F8);

        static auto Cbuf_AddText = reinterpret_cast<void (*)(int localClientNum, const char *text)>(0x8224D8E0);

        static auto ClientScr_ReadOnly = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BC6F8);
        static auto ClientScr_GetName = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BC730);
        static auto ClientScr_SetScore = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BC938);
        static auto ClientScr_SetSessionState = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BC7A0);
        static auto ClientScr_GetSessionState = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BC8B0);
        static auto ClientScr_SetStatusIcon = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCA40);
        static auto ClientScr_GetStatusIcon = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCA80);
        static auto ClientScr_SetSpectatorClient = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BC9D8);
        static auto ClientScr_SetArchiveTime = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCBB0);
        static auto ClientScr_GetArchiveTime = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCC00);
        static auto ClientScr_SetPSOffsetTime = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCC48);
        static auto ClientScr_GetPSOffsetTime = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCC80);
        static auto ClientScr_SetHeadIcon = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCAD0);
        static auto ClientScr_GetHeadIcon = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCB30);

        static auto Com_Printf = reinterpret_cast<void (*)(int channel, const char *fmt, ...)>(0x8224F7A0);
        static auto Com_PrintError = reinterpret_cast<void (*)(int channel, const char *fmt, ...)>(0x8224F8C0);
        static auto Com_PrintMessage = reinterpret_cast<void (*)(int channel, const char *msg, int error)>(0x8224F804);
        static auto CL_ConsolePrint = reinterpret_cast<void (*)(int localClientNum, int channel, const char *txt, int duration, int pixelWidth, int flags)>(0x8214EA60);

        static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x822862F8);

        static auto Scr_AddInt = reinterpret_cast<void (*)(int value, scriptInstance_t inst)>(0x82342000);
        static auto Scr_GetInt = reinterpret_cast<int (*)(unsigned int index, scriptInstance_t inst)>(0x823479F8);
        static auto Scr_AddSourceBuffer = reinterpret_cast<char *(*)(scriptInstance_t inst, const char *filename, const char *extFilename, const char *codePos, bool archive)>(0x82334558);
        static auto Scr_Error = reinterpret_cast<void (*)(const char *error, scriptInstance_t inst)>(0x823489A8);
        static auto Scr_GetMethod = reinterpret_cast<BuiltinMethod (*)(const char **pName, int *type)>(0x821FE6F0);
        static auto Scr_GetFunction = reinterpret_cast<BuiltinFunction (*)(const char **pName, int *type)>(0x821FCA38);
        static auto Player_GetMethod = reinterpret_cast<BuiltinMethod (*)(const char **pName)>(0x821C39B8);
        static auto Scr_GetNumParam = reinterpret_cast<unsigned int (*)(scriptInstance_t inst)>(0x82341FE8);
        static auto Scr_ObjectError = reinterpret_cast<void (*)(const char *error, scriptInstance_t inst)>(0x823428F8);
        static auto Scr_GetString = reinterpret_cast<const char *(*)(unsigned int index, scriptInstance_t inst)>(0x823480F0);
        static auto Scr_ParamError = reinterpret_cast<void (*)(unsigned int index, const char *error, scriptInstance_t inst)>(0x82342878);
        static auto GetObjectType = reinterpret_cast<unsigned int (*)(unsigned int id, scriptInstance_t inst)>(0x82339DC8);
        static auto Scr_GetEntity = reinterpret_cast<gentity_s *(*)(unsigned int index)>(0x82211A08);

        static auto GScr_AddFieldsForClient = reinterpret_cast<void (*)()>(0x821BCC90);
        static auto Scr_AddClassField = reinterpret_cast<void (*)(unsigned int classnum, const char *name, unsigned __int16 offset, scriptInstance_t inst)>(0x8233DE38);

        static auto Scr_SetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs, scriptInstance_t inst)>(0x82212D38);
        static auto Scr_GetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs, scriptInstance_t inst, unsigned int whichbits)>(0x82212FA8);

        static auto Scr_SetClientField = reinterpret_cast<void (*)(gclient_s *client, int offset, scriptInstance_t inst)>(0x821BCCF0);
        static auto Scr_GetObjectField = reinterpret_cast<void (*)(unsigned int classnum, int entnum, int offset, scriptInstance_t inst)>(0x82213670);

        static auto GScr_AllocString = reinterpret_cast<int (*)(const char *s)>(0x821E2CE0);

        static auto SV_LinkEntity = reinterpret_cast<void (*)(gentity_s *gEnt)>(0x8225DB98);
        static auto SV_UnlinkEntity = reinterpret_cast<void (*)(gentity_s *gEnt)>(0x8225DB50);
        static auto SV_SetBrushModel = reinterpret_cast<void (*)(gentity_s *ent)>(0x8225CC20);
        static auto SV_LocateGameData = reinterpret_cast<void (*)(gentity_s *gEnts, int numGEntities, int sizeofGEntity_t, playerState_s *clients, int sizeofGameClient)>(0x8225CCA8);

        static auto Load_clipMap_t = reinterpret_cast<void (*)(bool atStreamStart)>(0x82165290);

        static auto UI_Refresh = reinterpret_cast<void (*)(int localClientNum)>(0x8226B7D0);

        struct Font_s;

        struct ScreenPlacement;
        static auto scrPlaceFullUnsafe = reinterpret_cast<ScreenPlacement *>(0x8253F1D0);

        static auto UI_DrawText = reinterpret_cast<void (*)(const ScreenPlacement *scrPlace,
                                                            const char *text,
                                                            int maxChars,
                                                            Font_s *font,
                                                            float x,
                                                            float y,
                                                            int horzAlign,
                                                            int vertAlign,
                                                            float scale,
                                                            const float *color,
                                                            int style)>(0x82266F28);

        struct XAssetType;
        struct XAssetHeader;

        static auto DB_FindXAssetHeader = reinterpret_cast<XAssetHeader (*)(XAssetType type, const char *name, bool errorIfMissing, int waitTime)>(0x8216B688);

        // imageTrack is unused internally, it seems to be remnant from `useFastFile` dvar
        // dev build could load raw font objects
        static auto R_RegisterFont = reinterpret_cast<Font_s *(*)(const char *name, int imageTrack)>(0x8242B460);

        static auto va = reinterpret_cast<char *(*)(char *format, ...)>(0x82294218);

    }
}
