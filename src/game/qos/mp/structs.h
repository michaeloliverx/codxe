#pragma once

namespace qos
{
namespace mp
{
struct scr_entref_t
{
    uint16_t entnum;
    uint16_t classnum;
};

typedef void (*BuiltinFunction)();

struct BuiltinFunctionDef
{
    const char *actionString;
    BuiltinFunction actionFunc;
    int type;
};

typedef void (*BuiltinMethod)(scr_entref_t entref);

struct BuiltinMethodDef
{
    const char *actionString;
    void (*actionFunc)(scr_entref_t);
    int type;
};

struct gclient_s
{
    char pad_0[13252];
    int noclip;
    int ufo;
};
static_assert(offsetof(gclient_s, noclip) == 13252, "");
static_assert(offsetof(gclient_s, ufo) == 13256, "");

struct gentity_s
{
    char pad_0[388];
    gclient_s *client;
    char pad_392[36];
    int flags;
    char pad_428[224];
};
static_assert(sizeof(gentity_s) == 656, "");
static_assert(offsetof(gentity_s, client) == 388, "");
static_assert(offsetof(gentity_s, flags) == 428, "");

struct cplane_s
{
    float normal[3];
    float dist;
    unsigned __int8 type;
    unsigned __int8 signbits;
    unsigned __int8 pad[2];
};

struct __declspec(align(2)) cbrushside_t
{
    cplane_s *plane;
    unsigned int materialNum;
    __int16 firstAdjacentSideOffset;
    unsigned __int8 edgeCount;
};

#pragma warning(disable : 4324)

struct __declspec(align(16)) cbrush_t
{
    float mins[3];
    int contents;
    float maxs[3];
    unsigned int numsides;
    cbrushside_t *sides;
    __int16 axialMaterialNum[2][3];
    unsigned __int8 *baseAdjacentSide;
    __int16 firstAdjacentSideOffsets[2][3];
    unsigned __int8 edgeCount[2][3];
};

#pragma warning(default : 4324)

static_assert(sizeof(cbrush_t) == 80, "");

struct clipMap_t
{
    const char *name;
    int isInUse;
    char pad[140];
    int numSubModels;
    int *cmodels;
    unsigned __int16 numBrushes;
    cbrush_t *brushes;
};
static_assert(offsetof(clipMap_t, name) == 0, "");
static_assert(offsetof(clipMap_t, isInUse) == 4, "");
static_assert(offsetof(clipMap_t, numSubModels) == 148, "");
static_assert(offsetof(clipMap_t, cmodels) == 152, "");
static_assert(offsetof(clipMap_t, numBrushes) == 156, "");
static_assert(offsetof(clipMap_t, brushes) == 160, "");

} // namespace mp
} // namespace qos
