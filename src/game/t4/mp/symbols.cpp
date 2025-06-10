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

        R_AddCmdDrawText_t R_AddCmdDrawText = reinterpret_cast<R_AddCmdDrawText_t>(0x82401C30);

        UI_DrawBuildNumber_t UI_DrawBuildNumber = reinterpret_cast<UI_DrawBuildNumber_t>(0x8229BCF8);
    }
}
