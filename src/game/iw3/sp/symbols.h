#pragma once

#include "structs.h"

namespace iw3
{
    namespace sp
    {
        static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x82192CC0);
        static auto Scr_AddSourceBuffer = reinterpret_cast<char *(*)(const char *filename, const char *extFilename, const char *codePos, bool archive)>(0x821C5A18);
    }
}
