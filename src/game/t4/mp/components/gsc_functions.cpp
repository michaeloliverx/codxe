#include "gsc_functions.h"
#include "common.h"

namespace t4
{
namespace mp
{

/**
 * Checks if a 3D point is contained within an axis-aligned bounding box
 */
bool IsPointInBounds(const float mins[3], const float maxs[3], const float point[3])
{
    return (point[0] >= mins[0] && point[0] <= maxs[0]) && (point[1] >= mins[1] && point[1] <= maxs[1]) &&
           (point[2] >= mins[2] && point[2] <= maxs[2]);
}

void GSCrGetPlayerclipBrushesContainingPoint()
{
    float point[3] = {0};
    Scr_GetVector(0, point, SCRIPTINSTANCE_SERVER, -1);

    std::vector<int> brushIndices;
    for (int i = 0; i < cm->numBrushes; ++i)
    {
        auto &brush = cm->brushes[i];
        if (brush.contents & 0x10000 /* CONTENTS_PLAYERCLIP */ && IsPointInBounds(brush.mins, brush.maxs, point))
            brushIndices.push_back(i);
    }

    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    for (size_t i = 0; i < brushIndices.size(); ++i)
    {
        Scr_AddInt(brushIndices[i], SCRIPTINSTANCE_SERVER);
        Scr_AddArray(SCRIPTINSTANCE_SERVER);
    }
}

static struct
{
    const char *name;
    BuiltinFunction handler;
} gsc_functions[] = {
    {"getplayerclipbrushescontainingpoint", GSCrGetPlayerclipBrushesContainingPoint}, {nullptr, nullptr} // Terminator
};

Detour Scr_GetFunction_Detour;

BuiltinFunction Scr_GetFunction_Hook(const char **pName, int *type)
{
    if (pName != nullptr)
    {
        const auto *func = gsc_functions;
        for (; func->name != nullptr; ++func)
        {
            if (_stricmp(*pName, func->name) == 0)
                return func->handler;
        }
    }
    return Scr_GetFunction_Detour.GetOriginal<decltype(&Scr_GetFunction_Hook)>()(pName, type);
}

GSCFunctions::GSCFunctions()
{
    Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
    Scr_GetFunction_Detour.Install();
}

GSCFunctions::~GSCFunctions()
{
    Scr_GetFunction_Detour.Remove();
}
} // namespace mp
} // namespace t4
