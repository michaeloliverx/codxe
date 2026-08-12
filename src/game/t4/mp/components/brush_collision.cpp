#include "pch.h"
#include "common/brush_collision_tracker.h"
#include "brush_collision.h"

namespace t4
{
namespace mp
{
dvar_s *noclip_brushes = nullptr;

void RemoveBrushCollision(unsigned int index)
{
    if (cm->brushes[index].contents & 0x10000)
    {
        brush_collision_tracker::MarkModified(index);
        cm->brushes[index].contents &= ~0x10000;
    }
}

void RestoreBrushContents()
{
    if (!cm->isInUse)
    {
        DbgPrint("RestoreBrushContents: cm is not in use\n");
        return;
    }

    for (unsigned int i = 0; i < cm->numBrushes; ++i)
    {
        if (brush_collision_tracker::WasModified(i))
        {
            cm->brushes[i].contents |= 0x10000;
        }
    }

    brush_collision_tracker::Clear();
}

void RemoveAllBrushCollision()
{
    if (!cm->isInUse)
    {
        DbgPrint("RemoveAllBrushCollision: cm is not in use\n");
        return;
    }

    for (unsigned int i = 0; i < cm->numBrushes; ++i)
    {
        RemoveBrushCollision(i);
    }
}

Detour CM_LoadMap_Detour;
void CM_LoadMap_Hook(const char *name)
{
    // Let the clip map load first
    CM_LoadMap_Detour.GetOriginal<decltype(CM_LoadMap)>()(name);

    brush_collision_tracker::Clear();
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

void HandleBrushCollisionChange()
{
    if (R_CheckDvarModified(noclip_brushes))
    {
        // TODO: find out why noclip_brushes->current.string isn't working
        auto value = noclip_brushes->current.string;

        // Empty string means reset to original contents
        if (strcmp(value, "") == 0)
        {
            RestoreBrushContents();
            CG_GameMessage(0, "Brush collision reset");
        }
        // "*" means disable collision for all brushes
        else if (strcmp(value, "*") == 0)
        {
            RemoveAllBrushCollision();
            CG_GameMessage(0, "Brush collision disabled for all brushes");
        }
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

Detour CG_DrawActive_Detour;

void CG_DrawActive_Hook(int localClientNum)
{
    HandleBrushCollisionChange();
    CG_DrawActive_Detour.GetOriginal<decltype(CG_DrawActive)>()(localClientNum);
}

void BrushCollision::OnDvarInit()
{
    noclip_brushes = Dvar_RegisterString("noclip_brushes", "", DVAR_CODINFO,
                                         "Space separated list of brushes to disable collision on.");
}

BrushCollision::BrushCollision()
{
    CG_DrawActive_Detour = Detour(CG_DrawActive, CG_DrawActive_Hook);
    CG_DrawActive_Detour.Install();

    CM_LoadMap_Detour = Detour(CM_LoadMap, CM_LoadMap_Hook);
    CM_LoadMap_Detour.Install();
}

BrushCollision::~BrushCollision()
{

    CG_DrawActive_Detour.Remove();
    CM_LoadMap_Detour.Remove();
}
} // namespace mp
} // namespace t4
