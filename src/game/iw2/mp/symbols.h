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

typedef void (*Cbuf_AddText_t)(const char *text);
static Cbuf_AddText_t Cbuf_AddText = reinterpret_cast<Cbuf_AddText_t>(0x82370100);

static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x8230DFB8);
static auto Scr_AddSourceBuffer =
    reinterpret_cast<char *(*)(const char *filename, const char *extFilename, const char *codePos, bool archive)>(
        0x82354250);

static auto Scr_GetString = reinterpret_cast<char *(*)(unsigned int index)>(0x82345418);

typedef gentity_s *(*GetPlayerEntity_t)(scr_entref_t entref);
static GetPlayerEntity_t GetPlayerEntity = reinterpret_cast<GetPlayerEntity_t>(0x823CCC28);

// TODO: verify this one
typedef void (*Scr_Error_t)(const char *error);
static auto Scr_Error = reinterpret_cast<Scr_Error_t>(0x82342AC0);

typedef void (*Scr_AddBool_t)(int value);
static auto Scr_AddBool = reinterpret_cast<Scr_AddBool_t>(0x82346948);

typedef void (*Scr_AddInt_t)(int value);
static auto Scr_AddInt = reinterpret_cast<Scr_AddInt_t>(0x82346900);

typedef int (*Key_StringToKeynum_t)(const char *str);
static Key_StringToKeynum_t Key_StringToKeynum = reinterpret_cast<Key_StringToKeynum_t>(0x8241E338);

static auto va = reinterpret_cast<char *(*)(char *format, ...)>(0x82304090);
} // namespace mp
} // namespace iw2
