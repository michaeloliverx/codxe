#include "pch.h"
#include "events.h"
#include "clipmap.h"

namespace iw3
{
namespace mp
{
dvar_s *noclip_brushes = nullptr;

static std::array<uint32_t, USHRT_MAX + 1> brushContents;

void SaveBrushContents()
{
    assert(cm->isInUse);
    assert(static_cast<size_t>(cm->numBrushes) <= brushContents.size());

    for (int i = 0; i < cm->numBrushes; ++i)
    {
        brushContents[i] = cm->brushes[i].contents;
    }
}

void RestoreBrushContents()
{
    assert(cm->isInUse);
    assert(static_cast<size_t>(cm->numBrushes) <= brushContents.size());

    for (int i = 0; i < cm->numBrushes; ++i)
    {
        cm->brushes[i].contents = brushContents[i];
    }
}

void RemoveAllBrushCollision()
{
    assert(cm->isInUse);

    for (int i = 0; i < cm->numBrushes; ++i)
    {
        cm->brushes[i].contents &= ~CONTENTS_PLAYERCLIP; // Disable collision for all brushes
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
                cm->brushes[idx].contents &= ~CONTENTS_PLAYERCLIP; // Disable collision
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
    Events::OnCG_Init(SaveBrushContents);
    Events::OnCG_DrawActive(clipmap::HandleBrushCollisionChange);
}

clipmap::~clipmap()
{
}
} // namespace mp
} // namespace iw3
