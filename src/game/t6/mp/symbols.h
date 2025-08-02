#pragma once

#include "structs.h"

namespace t6
{
    namespace mp
    {
        static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x8248F1B0);

        static auto Scr_AddSourceBuffer = reinterpret_cast<char *(*)(scriptInstance_t inst, const char *filename, const char *extFilename, const char *codePos, bool archive)>(0x82530128);

        struct ScriptParseTree
        {
            const char *name;
            int len;
            char *buffer;
        };

        static auto Load_ScriptParseTreeAsset = reinterpret_cast<void (*)(ScriptParseTree **a1)>(0x822CA4C8);

    }
}
