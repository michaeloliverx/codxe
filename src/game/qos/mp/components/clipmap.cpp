#include "pch.h"
#include "events.h"
#include "g_scr_main.h"
#include "clipmap.h"

static const int SURFACE_FLAG_PLAYERCLIP = 0x10000;

namespace qos
{
namespace mp
{

dvar_s *clipmap::noclip_brushes;
std::vector<int> clipmap::brushContents;

void clipmap::SaveBrushContents()
{
    if (!cm->isInUse)
        return;

    brushContents.resize(cm->numBrushes, 0);
    for (int i = 0; i < cm->numBrushes; ++i)
    {
        brushContents[i] = cm->brushes[i].contents;
    }
}

void clipmap::RestoreBrushContents()
{
    if (!cm->isInUse)
        return;

    if (brushContents.size() != static_cast<size_t>(cm->numBrushes))
    {
        DbgPrint("RestoreBrushContents: size mismatch - saved: %u, current: %d\n",
                 static_cast<unsigned int>(brushContents.size()), cm->numBrushes);
        return;
    }

    for (int i = 0; i < cm->numBrushes; ++i)
    {
        cm->brushes[i].contents = brushContents[i];
    }
}

void clipmap::RemoveAllBrushesContents()
{
    if (!cm->isInUse)
        return;

    for (int i = 0; i < cm->numBrushes; ++i)
    {
        cm->brushes[i].contents &= ~SURFACE_FLAG_PLAYERCLIP;
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

    for (int i = 0; i < cm->numBrushes; ++i)
    {
        if ((brushContents[i] & SURFACE_FLAG_PLAYERCLIP) && !(cm->brushes[i].contents & SURFACE_FLAG_PLAYERCLIP))
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
            cm->brushes[idx].contents &= ~SURFACE_FLAG_PLAYERCLIP;
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
            brush->contents &= ~SURFACE_FLAG_PLAYERCLIP;
    }

    RebuildNoclipBrushesDvar();
}

clipmap::clipmap()
{

    Events::OnCG_Init(
        []()
        {
            clipmap::RegisterDvars();
            SaveBrushContents();
        });

    Events::OnCG_DrawActive(
        []()
        {
            if (R_CheckDvarModified(clipmap::noclip_brushes))
            {
                HandleclipmapChange();
            }
        });

    Scr_AddMethod("disableplayerclipontouchingbrushes", PlayerCmd_DisablePlayerClipOnTouchingBrushes, BUILTIN_ANY);
}

clipmap::~clipmap()
{
}
} // namespace mp
} // namespace qos
