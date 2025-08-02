#pragma once

#include "structs.h"

namespace qos
{
    namespace sp
    {
        static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x822DFD38);
        static auto Scr_AddSourceBuffer = reinterpret_cast<char *(*)(const char *filename, const char *extFilename, const char *codePos, bool archive)>(0x822ABF00);
    }
}
