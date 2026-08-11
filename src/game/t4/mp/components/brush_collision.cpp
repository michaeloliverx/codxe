#include "pch.h"
#include "brush_collision.h"
#include "events.h"

namespace t4
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
    if (cm->brushes[index].contents & 0x10000)
    {
        modifiedBrushes[index / BRUSHES_PER_WORD] |= 1u << (index % BRUSHES_PER_WORD);
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
        if (WasBrushModified(i))
        {
            cm->brushes[i].contents |= 0x10000;
        }
    }

    ClearModifiedBrushes();
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

    ClearModifiedBrushes();
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

BrushCollision::BrushCollision()
{
    Events::OnDvarInit(
        []
        {
            noclip_brushes = Dvar_RegisterString("noclip_brushes", "", DVAR_CODINFO,
                                                 "Space separated list of brushes to disable collision on.");
        });

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
