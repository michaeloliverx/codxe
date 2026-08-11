#include "pch.h"
#include "common/brush_collision_tracker.h"
#include "clipmap.h"
#include "events.h"

iw4::mp_tu6::dvar_t *noclip_brushes = nullptr;

void RemoveBrushCollision(unsigned int index)
{
    if (iw4::mp_tu6::cm->brushContents[index] & CONTENTS_PLAYERCLIP)
    {
        brush_collision_tracker::MarkModified(index);
        iw4::mp_tu6::cm->brushContents[index] &= ~CONTENTS_PLAYERCLIP;
    }
}

Detour DB_LinkXAssetEntry1_Detour;

iw4::mp_tu6::XAssetEntryPoolEntry *DB_LinkXAssetEntry1_Hook(iw4::mp_tu6::XAssetType type,
                                                            iw4::mp_tu6::XAssetHeader *header)
{
    // Register once
    // TODO: Move to dvar init event
    if (!noclip_brushes)
    {
        noclip_brushes = iw4::mp_tu6::Dvar_RegisterString(
            "noclip_brushes", "", 0x10, "Brush indices to disable playerclip. Use '*' for all, '' to restore");
    }

    iw4::mp_tu6::XAssetEntryPoolEntry *entry =
        DB_LinkXAssetEntry1_Detour.GetOriginal<decltype(iw4::mp_tu6::DB_LinkXAssetEntry1)>()(type, header);

    if (type == iw4::mp_tu6::ASSET_TYPE_CLIPMAP_MP)
    {
        brush_collision_tracker::Clear();
    }

    return entry;
}

void RestoreBrushContents()
{
    assert(iw4::mp_tu6::cm->isInUse);

    for (unsigned int i = 0; i < iw4::mp_tu6::cm->numBrushes; ++i)
    {
        if (brush_collision_tracker::WasModified(i))
        {
            iw4::mp_tu6::cm->brushContents[i] |= CONTENTS_PLAYERCLIP;
        }
    }

    brush_collision_tracker::Clear();
}

void RemoveAllBrushCollision()
{
    assert(iw4::mp_tu6::cm->isInUse);

    for (unsigned int i = 0; i < iw4::mp_tu6::cm->numBrushes; ++i)
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

bool BoundsIntersect(const iw4::mp_tu6::Bounds &a, const iw4::mp_tu6::Bounds &b)
{
    // Check intersection in all three dimensions
    for (int i = 0; i < 3; i++)
    {
        // Distance between centers
        float centerDistance = std::abs(a.midPoint[i] - b.midPoint[i]);

        // Sum of half sizes
        float combinedHalfSize = a.halfSize[i] + b.halfSize[i];

        // If distance between centers is greater than combined half sizes,
        // they don't intersect in this dimension
        if (centerDistance > combinedHalfSize)
        {
            return false;
        }
    }

    // If we get here, they intersect in all dimensions
    return true;
}

void DisablePlayerClipOnIntersectingBrushes(iw4::mp_tu6::scr_entref_t entref)
{
    const iw4::mp_tu6::gentity_s *ent = iw4::mp_tu6::GetEntity(entref);

    std::vector<int> intersecting_brushes;
    for (int i = 0; i < iw4::mp_tu6::cm->numBrushes; ++i)
    {
        if ((iw4::mp_tu6::cm->brushContents[i] & CONTENTS_PLAYERCLIP) &&
            BoundsIntersect(ent->r.absBox, iw4::mp_tu6::cm->brushBounds[i]))
        {
            intersecting_brushes.push_back(i);
        }
    }

    if (intersecting_brushes.empty())
    {
        iw4::mp_tu6::CG_GameMessage(0, "^1No brushes with collision found at this point");
        return;
    }

    std::string new_value = noclip_brushes->current.string;
    for (size_t i = 0; i < intersecting_brushes.size(); ++i)
    {
        if (!new_value.empty())
            new_value += " ";
        new_value += std::to_string(static_cast<unsigned long long>(intersecting_brushes[i]));
    }

    iw4::mp_tu6::Dvar_SetString(noclip_brushes, new_value.c_str());

    iw4::mp_tu6::CG_GameMessage(0, iw4::mp_tu6::va("^2Disabled collision for brushes: %s", new_value.c_str()));
}

clipmap::clipmap()
{
    DB_LinkXAssetEntry1_Detour = Detour(iw4::mp_tu6::DB_LinkXAssetEntry1, DB_LinkXAssetEntry1_Hook);
    DB_LinkXAssetEntry1_Detour.Install();

    Events::OnCG_DrawActive(
        []()
        {
            if (iw4::mp_tu6::R_CheckDvarModified(noclip_brushes))
            {
                assert(iw4::mp_tu6::cm->isInUse);

                std::string value = noclip_brushes->current.string;

                if (value == "")
                {
                    RestoreBrushContents();
                }
                else if (value == "*")
                {
                    RemoveAllBrushCollision();
                }
                else
                {
                    RestoreBrushContents();
                    const auto brush_indices = ParseSpaceSeparatedInts(value);
                    for (size_t i = 0; i < brush_indices.size(); ++i)
                    {
                        const int idx = brush_indices[i];
                        if (idx < 0 || idx >= iw4::mp_tu6::cm->numBrushes)
                        {
                            iw4::mp_tu6::CG_GameMessage(
                                0, iw4::mp_tu6::va("^1Error: Invalid brush index %d for map", idx));
                            continue;
                        }
                        RemoveBrushCollision(idx);
                    }
                }
            }
        });
}

clipmap::~clipmap()
{
    DB_LinkXAssetEntry1_Detour.Remove();
}
