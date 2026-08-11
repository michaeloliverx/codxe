#pragma once

namespace ngl
{
namespace mp
{

enum CvarFlags
{
    CVAR_SYSTEMINFO = 0x8,
};

struct cvar_t
{
    const char *name;
    const char *string;
    const char *resetString;
    const char *latchedString;
    int flags;
    int modified;
    int modificationCount;
    float value;
    int integer;
    cvar_t *next;
    cvar_t *hashNext;
};
static_assert(sizeof(cvar_t) == 0x2C, "");
static_assert(offsetof(cvar_t, integer) == 0x20, "");

} // namespace mp
} // namespace ngl
