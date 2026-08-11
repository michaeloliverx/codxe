#include "pch.h"
#include "events.h"
#include "clipmap.h"

namespace iw3
{
namespace mp
{
dvar_s *noclip_brushes = nullptr;

enum
{
    MAX_BRUSH_COUNT = USHRT_MAX + 1,
    BRUSHES_PER_WORD = sizeof(uint32_t) * 8,
};
static uint32_t modifiedBrushes[MAX_BRUSH_COUNT / BRUSHES_PER_WORD];

void ClearModifiedBrushes()
{
    memset(modifiedBrushes, 0, sizeof(modifiedBrushes));
}

bool WasBrushModified(unsigned int index)
{
    return (modifiedBrushes[index / BRUSHES_PER_WORD] & (1u << (index % BRUSHES_PER_WORD))) != 0;
}

void RemoveBrushCollision(unsigned int index)
{
    if (cm->brushes[index].contents & CONTENTS_PLAYERCLIP)
    {
        modifiedBrushes[index / BRUSHES_PER_WORD] |= 1u << (index % BRUSHES_PER_WORD);
        cm->brushes[index].contents &= ~CONTENTS_PLAYERCLIP;
    }
}

void RestoreBrushContents()
{
    assert(cm->isInUse);
    assert(static_cast<size_t>(cm->numBrushes) <= MAX_BRUSH_COUNT);

    for (unsigned int i = 0; i < cm->numBrushes; ++i)
    {
        if (WasBrushModified(i))
        {
            cm->brushes[i].contents |= CONTENTS_PLAYERCLIP;
        }
    }

    ClearModifiedBrushes();
}

void RemoveAllBrushCollision()
{
    assert(cm->isInUse);

    for (unsigned int i = 0; i < cm->numBrushes; ++i)
    {
        RemoveBrushCollision(i);
    }
}

std::vector<int> ParseSpaceSeparatedInts(const std::string &str)
{
    std::vector<int> result;
    std::istringstream iss(str);
    int value;

    while (iss >> value)
    {
        result.push_back(value);
    }

    return result;
}

void clipmap::HandleBrushCollisionChange()
{
    if (R_CheckDvarModified(noclip_brushes))
    {
        auto value = noclip_brushes->current.string;

        // Empty string means reset to original contents
        if (strcmp(value, "") == 0)
            RestoreBrushContents();
        // "*" means disable collision for all brushes
        else if (strcmp(value, "*") == 0)
            RemoveAllBrushCollision();
        else
        {
            RestoreBrushContents();
            auto brushIndices = ParseSpaceSeparatedInts(value);
            for (size_t i = 0; i < brushIndices.size(); ++i)
            {
                auto idx = brushIndices[i];
                if (idx < 0 || idx >= cm->numBrushes)
                {
                    DbgPrint("Error: Invalid brush index: %d\n", idx);
                    continue;
                }
                RemoveBrushCollision(idx);
            }
        }
    }
}

clipmap::clipmap()
{
    Events::OnDvarInit(
        []
        {
            noclip_brushes = Dvar_RegisterString("noclip_brushes", "", DVAR_CODINFO,
                                                 "Space separated list of brushes to disable collision on.");
        });
    Events::OnCG_Init(ClearModifiedBrushes);
    Events::OnCG_DrawActive(clipmap::HandleBrushCollisionChange);
}

clipmap::~clipmap()
{
}
} // namespace mp
} // namespace iw3
