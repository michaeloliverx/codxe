#include "clipmap.h"
#include "common.h"
#include "g_scr_main.h"

namespace iw4
{
namespace mp
{
std::vector<int> brush_contents;

Detour DB_LinkXAssetEntry1_Detour;

XAssetEntryPoolEntry *DB_LinkXAssetEntry1_Hook(XAssetType type, XAssetHeader *header)
{
    XAssetEntryPoolEntry *entry = DB_LinkXAssetEntry1_Detour.GetOriginal<decltype(DB_LinkXAssetEntry1)>()(type, header);

    if (type == ASSET_TYPE_CLIPMAP_MP)
    {
        // Resize the vector to match the number of brushes
        brush_contents.resize(header->clipMap->numBrushes);

        // Save original contents
        for (int i = 0; i < header->clipMap->numBrushes; ++i)
        {
            brush_contents[i] = header->clipMap->brushContents[i]; // Assuming this is the field you want to save
        }
    }

    return entry;
}

bool IsPointInBounds(float point[3], Bounds bounds)
{
    return (point[0] >= bounds.midPoint[0] - bounds.halfSize[0] &&
            point[0] <= bounds.midPoint[0] + bounds.halfSize[0]) &&
           (point[1] >= bounds.midPoint[1] - bounds.halfSize[1] &&
            point[1] <= bounds.midPoint[1] + bounds.halfSize[1]) &&
           (point[2] >= bounds.midPoint[2] - bounds.halfSize[2] && point[2] <= bounds.midPoint[2] + bounds.halfSize[2]);
}

void GScr_DisableBrushCollisionAtPoint()
{
    if (Scr_GetNumParam() != 1)
    {
        Scr_Error("DisableBrushCollisionAtPoint: Expected 1 argument (vector3 point)\n");
        return;
    }

    float point[3] = {0};
    Scr_GetVector(0, point);

    std::vector<int> modified_brushes;
    for (int i = 0; i < cm->numBrushes; ++i)
    {
        if ((cm->brushContents[i] & CONTENTS_PLAYERCLIP) && IsPointInBounds(point, cm->brushBounds[i]))
        {
            // Disable collision for this brush
            cm->brushContents[i] &= ~CONTENTS_PLAYERCLIP;
            modified_brushes.push_back(i);
        }
    }

    if (modified_brushes.empty())
    {
        CG_GameMessage(0, "^1No brushes with collision found at this point");
        return;
    }

    // Create message
    std::string message = "^2Disabled collision for brushes: ";
    for (size_t i = 0; i < modified_brushes.size(); ++i)
    {
        message += std::to_string(static_cast<unsigned long long>(modified_brushes[i]));
        if (i < modified_brushes.size() - 1)
            message += ", ";
    }
    CG_GameMessage(0, message.c_str());
}

void GScr_RestoreBrushCollision()
{
    assert(cm->isInUse);
    assert(brush_contents.size() == static_cast<size_t>(cm->numBrushes));

    // Restore original contents
    for (int i = 0; i < cm->numBrushes; ++i)
    {
        cm->brushContents[i] = brush_contents[i];
    }
}

clipmap::clipmap()
{
    DB_LinkXAssetEntry1_Detour = Detour(DB_LinkXAssetEntry1, DB_LinkXAssetEntry1_Hook);
    DB_LinkXAssetEntry1_Detour.Install();

    Scr_AddFunction("disablebrushcollisionatpoint", GScr_DisableBrushCollisionAtPoint, 0);
    Scr_AddFunction("restorebrushcollision", GScr_RestoreBrushCollision, 0);
}

clipmap::~clipmap()
{
    DB_LinkXAssetEntry1_Detour.Remove();
}
} // namespace mp
} // namespace iw4
