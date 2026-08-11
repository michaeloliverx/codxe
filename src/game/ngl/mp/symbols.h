#pragma once

#include "structs.h"

namespace ngl
{
namespace mp
{

typedef cvar_t *(*Cvar_Get_t)(const char *name, const char *defaultValue, int flags);
static Cvar_Get_t Cvar_Get = reinterpret_cast<Cvar_Get_t>(0x82514AF0);

// Called immediately after the inlined cvar initialization.
typedef void (*Cvar_RegisterCommands_t)();
static Cvar_RegisterCommands_t Cvar_RegisterCommands = reinterpret_cast<Cvar_RegisterCommands_t>(0x82517F80);

typedef float *(*PM_ClipVelocity_t)(const float *in, const float *normal, float *out);
static PM_ClipVelocity_t PM_ClipVelocity = reinterpret_cast<PM_ClipVelocity_t>(0x8244CFC8);

} // namespace mp
} // namespace ngl
