#pragma once

#include "structs.h"

namespace t5
{
namespace mp
{
static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x823DCEA8);

static auto Scr_AddSourceBuffer =
    reinterpret_cast<char *(*)(scriptInstance_t inst, const char *filename, const char *extFilename,
                               const char *codePos, bool archive)>(0x8245D5A0);
} // namespace mp
} // namespace t5
