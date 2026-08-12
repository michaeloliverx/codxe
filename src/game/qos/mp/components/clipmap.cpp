#include "pch.h"
#include "common/brush_collision_tracker.h"
#include "clipmap.h"

static const int SURFACE_FLAG_PLAYERCLIP = 0x10000;

namespace qos
{
namespace mp
{

dvar_s *clipmap::noclip_brushes;

void RemoveBrushCollision(unsigned int index)
{
    if (cm->brushes[index].contents & SURFACE_FLAG_PLAYERCLIP)
    {
        brush_collision_tracker::MarkModified(index);
        cm->brushes[index].contents &= ~SURFACE_FLAG_PLAYERCLIP;
    }
}

void clipmap::RestoreBrushContents()
{
    if (!cm->isInUse)
        return;

    for (unsigned int i = 0; i < cm->numBrushes; ++i)
    {
        if (brush_collision_tracker::WasModified(i))
        {
            cm->brushes[i].contents |= SURFACE_FLAG_PLAYERCLIP;
        }
    }

    brush_collision_tracker::Clear();
}

void clipmap::RemoveAllBrushesContents()
{
    if (!cm->isInUse)
        return;

    for (unsigned int i = 0; i < cm->numBrushes; ++i)
    {
        RemoveBrushCollision(i);
    }
}

std::vector<int> clipmap::ParseSpaceSeparatedInts(const std::string &str)
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

void clipmap::RebuildNoclipBrushesDvar()
{
    if (!cm->isInUse)
        return;

    std::ostringstream oss;
    bool first = true;

    for (unsigned int i = 0; i < cm->numBrushes; ++i)
    {
        if (brush_collision_tracker::WasModified(i))
        {
            if (!first)
                oss << " ";
            oss << i;
            first = false;
        }
    }

    Dvar_SetString(noclip_brushes, oss.str().c_str());
}

void clipmap::HandleclipmapChange()
{
    RestoreBrushContents();

    const std::string value = Dvar_GetString("noclip_brushes");

    if (value.empty())
    {
        CG_GameMessage(0, "Brush collision reset");
    }
    else if (value == "*")
    {
        RemoveAllBrushesContents();
        CG_GameMessage(0, "Brush collision disabled for all brushes");
    }
    else
    {
        std::vector<int> brushIndices = ParseSpaceSeparatedInts(value);
        for (size_t i = 0; i < brushIndices.size(); ++i)
        {
            int idx = brushIndices[i];
            if (idx < 0 || idx >= cm->numBrushes)
            {
                DbgPrint("[clipmap] Error: Invalid brush index: %d\n", idx);
                continue;
            }
            RemoveBrushCollision(idx);
        }
    }
}

void clipmap::RegisterDvars()
{
    static bool has_ran = false;
    if (has_ran)
        return;
    noclip_brushes = Dvar_RegisterString("noclip_brushes", "", DVAR_SERVERINFO,
                                         "Space separated list of brushes to disable collision on.");
    has_ran = true;
}

void clipmap::PlayerCmd_DisablePlayerClipOnTouchingBrushes(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);

    for (int i = 0; i < cm->numBrushes; ++i)
    {
        cbrush_t *brush = &cm->brushes[i];

        if (!(brush->contents & SURFACE_FLAG_PLAYERCLIP))
            continue;

        bool intersects = true;
        for (int j = 0; j < 3; ++j)
        {
            if (ent->absmax[j] < brush->mins[j] || ent->absmin[j] > brush->maxs[j])
            {
                intersects = false;
                break;
            }
        }

        if (intersects)
            RemoveBrushCollision(i);
    }

    RebuildNoclipBrushesDvar();
}

void clipmap::OnCGInit()
{
    RegisterDvars();
    brush_collision_tracker::Clear();
}

void clipmap::OnCGDrawActive()
{
    if (R_CheckDvarModified(noclip_brushes))
    {
        HandleclipmapChange();
    }
}

clipmap::clipmap()
{
}

clipmap::~clipmap()
{
}
} // namespace mp
} // namespace qos
