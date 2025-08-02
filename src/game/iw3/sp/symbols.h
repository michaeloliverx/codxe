#pragma once

#include "structs.h"

namespace iw3
{
    namespace sp

    {
        static auto DB_EnumXAssets_FastFile = reinterpret_cast<void *(*)(XAssetType type, void (*func)(void *asset, void *inData), void *inData, bool includeOverride)>(0x822AEF88);
        static auto DB_GetAllXAssetOfType_FastFile = reinterpret_cast<int (*)(XAssetType type, XAssetHeader *assets, int maxCount)>(0x822AEB20);

        static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x82192CC0);
        static auto Scr_AddSourceBuffer = reinterpret_cast<char *(*)(const char *filename, const char *extFilename, const char *codePos, bool archive)>(0x821C5A18);
    }
}
