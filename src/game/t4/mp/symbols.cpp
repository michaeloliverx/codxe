#include "common.h"
#include "symbols.h"

namespace t4
{
    namespace mp
    {
        clipMap_t *cm = reinterpret_cast<clipMap_t *>(0x82DD4F80);
        client_fields_s *client_fields = reinterpret_cast<client_fields_s *>(0x82026B80);
        gentity_s *g_entities = reinterpret_cast<gentity_s *>(0x82BAD1B0);

        DB_FindXAssetHeader_t DB_FindXAssetHeader = reinterpret_cast<DB_FindXAssetHeader_t>(0x821EA5D8);
        DB_GetAllXAssetOfType_FastFile_t DB_GetAllXAssetOfType_FastFile = reinterpret_cast<DB_GetAllXAssetOfType_FastFile_t>(0x821E7428);

        Hunk_AllocateTempMemoryHighInternal_t Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<Hunk_AllocateTempMemoryHighInternal_t>(0x822B9CC0);

        R_AddCmdDrawText_t R_AddCmdDrawText = reinterpret_cast<R_AddCmdDrawText_t>(0x82401C30);

        GScr_AddFieldsForClient_t GScr_AddFieldsForClient = reinterpret_cast<GScr_AddFieldsForClient_t>(0x8220A270);
        Scr_AddClassField_t Scr_AddClassField = reinterpret_cast<Scr_AddClassField_t>(0x823414F0);

        Scr_GetObjectField_t Scr_GetObjectField = reinterpret_cast<Scr_GetObjectField_t>(0x822556B0);
        Scr_SetClientField_t Scr_SetClientField = reinterpret_cast<Scr_SetClientField_t>(0x8220A2D0);

        Scr_GetGenericField_t Scr_GetGenericField = reinterpret_cast<Scr_GetGenericField_t>(0x82255100);
        Scr_SetGenericField_t Scr_SetGenericField = reinterpret_cast<Scr_SetGenericField_t>(0x82254E90);

        Scr_AddSourceBuffer_t Scr_AddSourceBuffer = reinterpret_cast<Scr_AddSourceBuffer_t>(0x82339EF8);

        UI_DrawBuildNumber_t UI_DrawBuildNumber = reinterpret_cast<UI_DrawBuildNumber_t>(0x8229BCF8);
    }
}
