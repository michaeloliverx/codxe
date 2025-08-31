#include "script.h"
#include "common.h"

namespace iw5
{
namespace mp
{

std::map<std::string, BuiltinFunction> scr_functions;
std::map<std::string, BuiltinMethod> scr_methods;

void Scr_AddFunction(const std::string &name, BuiltinFunction func)
{
    auto result = scr_functions.insert(std::make_pair(name, func));
    if (!result.second)
    {
        throw std::runtime_error("Function '" + name + "' already exists");
    }
}

void Scr_AddMethod(const std::string &name, const BuiltinMethod func)
{
    auto result = scr_methods.insert(std::make_pair(name, func));
    if (!result.second)
    {
        throw std::runtime_error("Method '" + name + "' already exists");
    }
}

void PlayerCmd_SetClientFlags(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    if (!ent->client)
        Scr_ErrorInternal();

    ent->client->flags = Scr_GetInt(1);
}

void PlayerCmd_GetClientFlags(scr_entref_t entref)
{
    const gentity_s *ent = GetEntity(entref);
    if (!ent->client)
        Scr_ErrorInternal();

    Scr_AddInt(ent->client->flags);
}

void PlayerCmd_SetEntityFlags(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    ent->flags = Scr_GetInt(1);
}

void PlayerCmd_GetEntityFlags(scr_entref_t entref)
{
    const gentity_s *ent = GetEntity(entref);
    Scr_AddInt(ent->flags);
}

void DisableBrushCollisionAtOrigin(scr_entref_t entref)
{
    if (!cm->isInUse)
    {
        DbgPrint("ERROR cm not in use\n");
        return;
    }

    const gentity_s *ent = GetEntity(entref);

    // Make a local copy of the origin
    float origin[3];
    origin[0] = ent->r.currentOrigin[0];
    origin[1] = ent->r.currentOrigin[1];
    origin[2] = ent->r.currentOrigin[2] - 1.0f; // Slightly lower to ensure we're within the brush

    std::vector<unsigned short> brush_indices;

    for (unsigned short i = 0; i < cm->info.numBrushes; i++)
    {
        const Bounds &bounds = cm->info.brushBounds[i];
        // Check if the point is inside the AABB
        if (
            // Check if the brush is a playerclip
            cm->info.brushContents[i] & CONTENTS_PLAYERCLIP &&
            // Check if the origin is within the bounds
            (origin[0] >= bounds.midPoint[0] - bounds.halfSize[0] &&
             origin[0] <= bounds.midPoint[0] + bounds.halfSize[0]) &&
            (origin[1] >= bounds.midPoint[1] - bounds.halfSize[1] &&
             origin[1] <= bounds.midPoint[1] + bounds.halfSize[1]) &&
            (origin[2] >= bounds.midPoint[2] - bounds.halfSize[2] &&
             origin[2] <= bounds.midPoint[2] + bounds.halfSize[2]))
        {
            brush_indices.push_back(i);
        }
    }

    if (brush_indices.empty())
    {
        CG_GameMessage(LOCAL_CLIENT_0, "^1No Brushes found at origin!");
        return;
    }

    std::string brush_indices_str = "^2Disabled playerclip on brushes ";
    for (unsigned short i = 0; i < brush_indices.size(); i++) // Fixed: initialize i to 0
    {
        cm->info.brushContents[brush_indices[i]] &= ~CONTENTS_PLAYERCLIP; // Fixed: use brush_indices[i] as index
        brush_indices_str += std::to_string(static_cast<unsigned long long>(brush_indices[i])) +
                             (i < brush_indices.size() - 1 ? ", " : "");
    }
    CG_GameMessage(LOCAL_CLIENT_0, brush_indices_str.c_str());
}

void RemoveBrushCollisions()
{
    // if (!cm->isInUse)
    // {
    //     DbgPrint("[PLUGIN] cm not in use\n");
    //     return;
    // }
    auto numBrushes = cm->info.numBrushes;
    DbgPrint("[PLUGIN] numBrushes=%d \n", numBrushes);

    for (unsigned short i = 0; i < cm->info.numBrushes; i++)
    {
        cm->info.brushContents[i] &= ~CONTENTS_PLAYERCLIP;
    }
}

Detour PlayerCmd_GetViewmodel_Detour;

void PlayerCmd_GetViewmodel_Hook(scr_entref_t entref)
{
    // DbgPrint("PlayerCmd_GetViewmodel_Hook called!\n");
    // DbgPrint("entref->entnum=%d entref->classnum=%d\n", entref.entnum, entref.classnum);

    // const char *s1 = Scr_GetString(0);
    // const std::string arg1 = (s1 ? std::string(s1) : std::string());
    // if (arg1 == "togglefo")
    // {
    //     DbgPrint("UFO command detected, calling original function\n");
    //     // gentity_s *ent = &g_entities[entref.entnum];

    //     gentity_s *ent = GetEntity(entref);
    //     if (ent->client == nullptr)
    //     {
    //         DbgPrint("ent->client is null\n");
    //         return;
    //     }
    //     // if (!(ent->client->flags & 2))
    //     // {
    //     //     DbgPrint("Enabling noclip\n");
    //     // }
    //     // else
    //     // {
    //     //     DbgPrint("Disabling noclip\n");
    //     // }

    //     ent->flags ^= FL_GODMODE;

    //     // ent->client->flags ^= 3; // PM_UFO

    //     unsigned char *clientPtr = (unsigned char *)(ent->client);
    //     *(int *)(clientPtr + 13820) ^= 2;

    //     // const int clientflags = ent->client->flags;
    //     // DbgPrint("ent->client->flags before UFO: %d\n", clientflags);

    //     // ent->client->flags ^= 2;

    //     return;
    // }

    // DbgPrint("cm->name %s\n", cm->name);
    // RemoveBrushCollisions();

    DbgPrint("PlayerCmd_GetViewmodel_Hook called!\n");

    const char *s1 = Scr_GetString(0);
    const std::string arg1 = (s1 ? std::string(s1) : std::string());
    DbgPrint("arg1=%s\n", arg1.c_str());
    auto it = scr_methods.find(arg1);
    if (it != scr_methods.end())
    {
        DbgPrint("Found function '%s', calling it\n", arg1.c_str());
        return it->second(entref);
    }

    PlayerCmd_GetViewmodel_Detour.GetOriginal<PlayerCmd_GetViewmodel_t>()(entref);
}

Script::Script()
{
    Scr_AddMethod("setclientflags", PlayerCmd_SetClientFlags);
    Scr_AddMethod("getclientflags", PlayerCmd_GetClientFlags);

    Scr_AddMethod("setentityflags", PlayerCmd_SetEntityFlags);
    Scr_AddMethod("getentityflags", PlayerCmd_GetEntityFlags);

    Scr_AddMethod("disablebrushcollisionatorigin", DisableBrushCollisionAtOrigin);

    PlayerCmd_GetViewmodel_Detour = Detour(PlayerCmd_GetViewmodel, PlayerCmd_GetViewmodel_Hook);
    PlayerCmd_GetViewmodel_Detour.Install();
}

Script::~Script()
{
    PlayerCmd_GetViewmodel_Detour.Remove();
}
} // namespace mp
} // namespace iw5
