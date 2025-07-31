#pragma once

#include "structs.h"

namespace iw3
{
    namespace sp
    {
        static auto cmd_functions = reinterpret_cast<cmd_function_s *>(0x82DDEFCC);

        static auto Cbuf_AddText = reinterpret_cast<void (*)(int localClientNum, const char *text)>(0x821DE9D8);

        static auto CL_ConsolePrint = reinterpret_cast<void (*)(int localClientNum, int channel, const char *txt, int duration, int pixelWidth, int flags)>(0x822D7040);
        static auto CL_GamepadButtonEvent = reinterpret_cast<void (*)(int localClientNum, int controllerIndex, int key, int down, unsigned int time)>(0x822CE630);

        static auto Cmd_AddCommandInternal = reinterpret_cast<void (*)(const char *cmdName, void (*function)(), cmd_function_s *allocedCmd)>(0x821DFAD0);

        static auto Com_Printf = reinterpret_cast<void (*)(conChannel_t channel, const char *fmt, ...)>(0x821DC0A0);
        static auto Com_PrintError = reinterpret_cast<void (*)(conChannel_t channel, const char *fmt, ...)>(0x821DAC90);
        static auto Com_PrintWarning = reinterpret_cast<void (*)(conChannel_t channel, const char *fmt, ...)>(0x821DA798);

        static auto DB_EnumXAssets_FastFile = reinterpret_cast<void *(*)(XAssetType type, void (*func)(void *asset, void *inData), void *inData, bool includeOverride)>(0x822AEF88);
        static auto DB_GetAllXAssetOfType_FastFile = reinterpret_cast<int (*)(XAssetType type, XAssetHeader *assets, int maxCount)>(0x822AEB20);

        static auto Load_MapEntsPtr = reinterpret_cast<void (*)()>(0x822A9648);

        static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x82192CC0);
        static auto Scr_AddSourceBuffer = reinterpret_cast<char *(*)(const char *filename, const char *extFilename, const char *codePos, bool archive)>(0x821C5A18);
    }
}