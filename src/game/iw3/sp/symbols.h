#pragma once

#include "structs.h"

namespace iw3
{
namespace sp
{
static auto cmd_functions = reinterpret_cast<cmd_function_s *>(0x82DDEFCC);

static auto Cbuf_AddText = reinterpret_cast<void (*)(int localClientNum, const char *text)>(0x821DE9D8);

static auto CL_ConsolePrint = reinterpret_cast<void (*)(int localClientNum, int channel, const char *txt, int duration,
                                                        int pixelWidth, int flags)>(0x822D7040);

static auto Cmd_AddCommandInternal =
    reinterpret_cast<void (*)(const char *cmdName, void (*function)(), cmd_function_s *allocedCmd)>(0x821DFAD0);

typedef void (*Cmd_Init_t)();
static Cmd_Init_t Cmd_Init = reinterpret_cast<Cmd_Init_t>(0x821DFF40);

typedef void (*Com_InitDvars_t)();
static Com_InitDvars_t Com_InitDvars = reinterpret_cast<Com_InitDvars_t>(0x821DBB90);

static auto Com_Printf = reinterpret_cast<void (*)(conChannel_t channel, const char *fmt, ...)>(0x821DC0A0);
static auto Com_PrintError = reinterpret_cast<void (*)(conChannel_t channel, const char *fmt, ...)>(0x821DAC90);
static auto Com_PrintWarning = reinterpret_cast<void (*)(conChannel_t channel, const char *fmt, ...)>(0x821DA798);

static auto DB_EnumXAssets_FastFile =
    reinterpret_cast<void *(*)(XAssetType type, void (*func)(void *asset, void *inData), void *inData,
                               bool includeOverride)>(0x822AEF88);
static auto DB_GetAllXAssetOfType_FastFile =
    reinterpret_cast<int (*)(XAssetType type, XAssetHeader *assets, int maxCount)>(0x822AEB20);

static auto Load_MapEntsPtr = reinterpret_cast<void (*)()>(0x822B9788);

static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x82192CC0);
static auto Scr_Error = reinterpret_cast<void (*)(const char *error)>(0x821B1740);
static auto Scr_GetFunction = reinterpret_cast<BuiltinFunction (*)(const char **pName, int *type)>(0x8222FD70);
static auto Scr_GetNumParam = reinterpret_cast<unsigned int (*)()>(0x821B13D0);
static auto Scr_GetString = reinterpret_cast<const char *(*)(unsigned int index)>(0x821B3488);
static auto Scr_AddSourceBuffer =
    reinterpret_cast<char *(*)(const char *filename, const char *extFilename, const char *codePos, bool archive)>(
        0x821C5A18);
} // namespace sp
} // namespace iw3
