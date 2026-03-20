#pragma once

#include "structs.h"

namespace iw2
{
namespace mp
{
static const unsigned int *scrVmPub_outparamcount = reinterpret_cast<const unsigned int *>(0x8447399C);

static auto g_entities = reinterpret_cast<gentity_s *>(0x83676880);
static qkey_t **keys = reinterpret_cast<qkey_t **>(0x824D3518);

static auto player_methods = reinterpret_cast<BuiltinMethodDef *>(0x8203EAC8);
static const int PLAYER_METHODS_COUNT = 59;

static const scr_const_t *scr_const = reinterpret_cast<scr_const_t *>(0x8358EF08);

typedef void (*Cbuf_AddText_t)(const char *text);
static Cbuf_AddText_t Cbuf_AddText = reinterpret_cast<Cbuf_AddText_t>(0x82370100);

typedef void *(*Hunk_AllocateTempMemoryHighInternal_t)(int size);
static Hunk_AllocateTempMemoryHighInternal_t Hunk_AllocateTempMemoryHighInternal =
    reinterpret_cast<Hunk_AllocateTempMemoryHighInternal_t>(0x8230DFB8);

typedef char *(*Scr_AddSourceBuffer_t)(const char *filename, const char *extFilename, const char *codePos,
                                       bool archive);
static Scr_AddSourceBuffer_t Scr_AddSourceBuffer = reinterpret_cast<Scr_AddSourceBuffer_t>(0x82354250);

typedef char *(*Scr_GetString_t)(unsigned int index);
static Scr_GetString_t Scr_GetString = reinterpret_cast<Scr_GetString_t>(0x82345418);

typedef gentity_s *(*GetPlayerEntity_t)(scr_entref_t entref);
static GetPlayerEntity_t GetPlayerEntity = reinterpret_cast<GetPlayerEntity_t>(0x823CCC28);

// TODO: verify this one
typedef void (*Scr_Error_t)(const char *error);
static auto Scr_Error = reinterpret_cast<Scr_Error_t>(0x82342AC0);

typedef void (*Scr_AddBool_t)(int value);
static auto Scr_AddBool = reinterpret_cast<Scr_AddBool_t>(0x82346948);

typedef void (*Scr_AddInt_t)(int value);
static auto Scr_AddInt = reinterpret_cast<Scr_AddInt_t>(0x82346900);

typedef void (*Scr_AddConstString_t)(int value);
static auto Scr_AddConstString = reinterpret_cast<Scr_AddConstString_t>(0x82346648);

typedef int (*Key_StringToKeynum_t)(const char *str);
static Key_StringToKeynum_t Key_StringToKeynum = reinterpret_cast<Key_StringToKeynum_t>(0x8241E338);

typedef BuiltinFunction (*Scr_GetFunction_t)(const char **pName, int *type);
static Scr_GetFunction_t Scr_GetFunction = reinterpret_cast<Scr_GetFunction_t>(0x823CB720);

typedef BuiltinMethod (*Scr_GetMethod_t)(const char **pName, int *type);
static Scr_GetMethod_t Scr_GetMethod = reinterpret_cast<Scr_GetMethod_t>(0x823CB968);

typedef unsigned int (*SL_GetStringOfLen_t)(const char *str, unsigned int user, unsigned int len, int type);
static auto SL_GetStringOfLen = reinterpret_cast<SL_GetStringOfLen_t>(0x82351F58);

typedef char *(*va_t)(char *format, ...);
static va_t va = reinterpret_cast<va_t>(0x82304090);
} // namespace mp
} // namespace iw2
