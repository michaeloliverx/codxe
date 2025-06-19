#pragma once

#include "structs.h"

namespace t4
{
    namespace sp
    {
        static auto g_entities = reinterpret_cast<gentity_s *>(0x8293A280);

        static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x822862F8);

        static auto Scr_AddInt = reinterpret_cast<void (*)(int value, scriptInstance_t inst)>(0x82342000);
        static auto Scr_AddSourceBuffer = reinterpret_cast<char *(*)(scriptInstance_t inst, const char *filename, const char *extFilename, const char *codePos, bool archive)>(0x82334558);
        static auto Scr_Error = reinterpret_cast<void (*)(const char *error, scriptInstance_t inst)>(0x823489A8);
        static auto Scr_GetMethod = reinterpret_cast<BuiltinMethod (*)(const char **pName, int *type)>(0x821FE6F0);
        static auto Scr_GetNumParam = reinterpret_cast<unsigned int (*)(scriptInstance_t inst)>(0x82341FE8);
        static auto Scr_ObjectError = reinterpret_cast<void (*)(const char *error, scriptInstance_t inst)>(0x823428F8);

        static auto va = reinterpret_cast<char *(*)(char *format, ...)>(0x82294218);
    }
}
