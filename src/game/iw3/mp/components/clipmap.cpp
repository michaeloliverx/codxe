#include "clipmap.h"
#include "common.h"

namespace iw3
{
namespace mp
{
dvar_s *noclip_brushes = nullptr;

std::vector<int> brushContents;

void SaveBrushContents()
{
    if (!cm->isInUse)
    {
        DbgPrint("SaveBrushContents: cm is not in use\n");
        return;
    }

    DbgPrint("SaveBrushContents: Saving brush contents for %d brushes\n", cm->numBrushes);

    brushContents.resize(cm->numBrushes, 0);
    for (int i = 0; i < cm->numBrushes; ++i)
    {
        brushContents[i] = cm->brushes[i].contents;
    }
}

void RestoreBrushContents()
{
    if (!cm->isInUse)
    {
        DbgPrint("RestoreBrushContents: cm is not in use\n");
        return;
    }

    if (brushContents.size() != static_cast<size_t>(cm->numBrushes))
    {
        DbgPrint("RestoreBrushContents: size mismatch - saved: %zu, current: %d\n", brushContents.size(),
                 cm->numBrushes);
        return;
    }

    for (int i = 0; i < cm->numBrushes; ++i)
    {
        cm->brushes[i].contents = brushContents[i];
    }
}

void RemoveAllBrushCollision()
{
    if (!cm->isInUse)
    {
        DbgPrint("RemoveAllBrushCollision: cm is not in use\n");
        return;
    }

    for (int i = 0; i < cm->numBrushes; ++i)
    {
        cm->brushes[i].contents &= ~CONTENTS_PLAYERCLIP; // Disable collision for all brushes
    }
}

Detour CM_LoadMap_Detour;

void CM_LoadMap_Hook(const char *name, unsigned int *checksum)
{
    // Let the clip map load first
    CM_LoadMap_Detour.GetOriginal<decltype(CM_LoadMap)>()(name, checksum);

    // Save the contents of the brushes
    SaveBrushContents();
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
    noclip_brushes = Dvar_RegisterString("noclip_brushes", "", DVAR_CODINFO,
                                         "Space separated list of brushes to disable collision on.");

    CM_LoadMap_Detour = Detour(CM_LoadMap, CM_LoadMap_Hook);
    CM_LoadMap_Detour.Install();
}

clipmap::~clipmap()
{
}
} // namespace mp
} // namespace iw3
