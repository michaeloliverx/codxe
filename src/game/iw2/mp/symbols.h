#pragma once

#include "structs.h"

namespace iw2
{
    namespace mp
    {
        static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x8230DFB8);
        static auto Scr_AddSourceBuffer = reinterpret_cast<char *(*)(const char *filename, const char *extFilename, const char *codePos, bool archive)>(0x82354250);
    }
}
