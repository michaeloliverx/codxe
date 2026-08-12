#include "pch.h"
#include "common/brush_collision_tracker.h"
#include "clipmap.h"

namespace iw3
{
namespace mp
{
dvar_s *noclip_brushes = nullptr;

void RemoveBrushCollision(unsigned int index)
{
    if (cm->brushes[index].contents & CONTENTS_PLAYERCLIP)
    {
        brush_collision_tracker::MarkModified(index);
        cm->brushes[index].contents &= ~CONTENTS_PLAYERCLIP;
    }
}

void RestoreBrushContents()
{
    assert(cm->isInUse);

    for (unsigned int i = 0; i < cm->numBrushes; ++i)
    {
        if (brush_collision_tracker::WasModified(i))
        {
            cm->brushes[i].contents |= CONTENTS_PLAYERCLIP;
        }
    }

    brush_collision_tracker::Clear();
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

void clipmap::OnDvarInit()
{
    noclip_brushes = Dvar_RegisterString("noclip_brushes", "", DVAR_CODINFO,
                                         "Space separated list of brushes to disable collision on.");
}

void clipmap::OnCGInit()
{
    brush_collision_tracker::Clear();
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

void clipmap::OnCGDrawActive()
{
    HandleBrushCollisionChange();
}

clipmap::clipmap()
{
}

clipmap::~clipmap()
{
}
} // namespace mp
} // namespace iw3
