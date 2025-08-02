#pragma once

#include "structs.h"

namespace t6
{
    namespace sp
    {
        static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x823733E8);

        static auto Scr_AddSourceBuffer = reinterpret_cast<char *(*)(scriptInstance_t inst, const char *filename, const char *extFilename, const char *codePos, bool archive)>(0x824267F0);
    }
}
