#include "pch.h"
#include "gsc.h"
#include "bots.h"
#include "common/gsc_registry.h"

namespace iw5
{
namespace mp
{

namespace
{
static const int MAX_SCRIPT_FILE_HANDLES = 8;
static const unsigned int GSC_TOKEN_OPENFILE = 0x99;
static const unsigned int GSC_TOKEN_CLOSEFILE = 0x9A;
static const unsigned int GSC_TOKEN_FPRINTLN = 0x9B;
static const unsigned int GSC_TOKEN_FREADLN = 0x9D;

struct ScriptFileHandle
{
    FILE *file;
};

ScriptFileHandle script_file_handles[MAX_SCRIPT_FILE_HANDLES];

std::string BuildScriptFilePath(const char *filename)
{
    std::string relative_path = filename ? filename : "";
    std::replace(relative_path.begin(), relative_path.end(), '/', '\\');

    const std::string mod_base_path = Config::GetModBasePath();
    if (mod_base_path.empty())
        return relative_path;

    return mod_base_path + "\\" + relative_path;
}

void EnsureParentDirectory(const std::string &path)
{
    char directory[MAX_PATH];
    strncpy(directory, path.c_str(), sizeof(directory) - 1);
    directory[sizeof(directory) - 1] = '\0';

    char *last_slash = strrchr(directory, '\\');
    if (last_slash)
    {
        *last_slash = '\0';
        filesystem::create_nested_dirs(directory);
    }
}

void CloseAllScriptFiles()
{
    for (int i = 0; i < MAX_SCRIPT_FILE_HANDLES; ++i)
    {
        if (script_file_handles[i].file)
        {
            fclose(script_file_handles[i].file);
            script_file_handles[i].file = nullptr;
        }
    }
}

void GScr_OpenFile()
{
    if (Scr_GetNumParam() != 2)
    {
        DbgPrint("[codxe][IW5][GSC] openfile expects 2 parameters\n");
        Scr_ErrorInternal();
        return;
    }

    const char *mode = Scr_GetString(1);
    const char *file_mode = nullptr;
    if (!_stricmp(mode, "read"))
        file_mode = "rt";
    else if (!_stricmp(mode, "write"))
        file_mode = "wt";
    else if (!_stricmp(mode, "append"))
        file_mode = "at";
    else
    {
        DbgPrint("[codxe][IW5][GSC] openfile received invalid mode '%s'\n", mode);
        Scr_AddInt(0);
        return;
    }

    const char *filename = Scr_GetString(0);
    const std::string path = BuildScriptFilePath(filename);
    if (file_mode[0] == 'w' || file_mode[0] == 'a')
        EnsureParentDirectory(path);

    for (int i = 0; i < MAX_SCRIPT_FILE_HANDLES; ++i)
    {
        if (!script_file_handles[i].file)
        {
            script_file_handles[i].file = fopen(path.c_str(), file_mode);
            if (!script_file_handles[i].file)
            {
                Scr_AddInt(0);
                return;
            }

            Scr_AddInt(i + 1);
            return;
        }
    }

    DbgPrint("[codxe][IW5][GSC] openfile exhausted its file handles\n");
    Scr_AddInt(0);
}

void GScr_CloseFile()
{
    if (Scr_GetNumParam() != 1)
    {
        DbgPrint("[codxe][IW5][GSC] closefile expects 1 parameter\n");
        Scr_ErrorInternal();
        return;
    }

    const int handle = Scr_GetInt(0);
    if (handle < 1 || handle > MAX_SCRIPT_FILE_HANDLES)
    {
        DbgPrint("[codxe][IW5][GSC] closefile received invalid handle %d\n", handle);
        Scr_AddInt(0);
        return;
    }

    ScriptFileHandle &slot = script_file_handles[handle - 1];
    if (!slot.file)
    {
        Scr_AddInt(0);
        return;
    }

    const int result = fclose(slot.file);
    slot.file = nullptr;
    Scr_AddInt(result == 0);
}

void GScr_ReadLine()
{
    if (Scr_GetNumParam() != 1)
    {
        DbgPrint("[codxe][IW5][GSC] freadln expects 1 parameter\n");
        Scr_ErrorInternal();
        return;
    }

    const int handle = Scr_GetInt(0);
    if (handle < 1 || handle > MAX_SCRIPT_FILE_HANDLES || !script_file_handles[handle - 1].file)
    {
        DbgPrint("[codxe][IW5][GSC] freadln received invalid handle %d\n", handle);
        Scr_AddUndefined();
        return;
    }

    char line[4096];
    if (!fgets(line, sizeof(line), script_file_handles[handle - 1].file))
    {
        Scr_AddUndefined();
        return;
    }

    size_t length = strlen(line);
    while (length && (line[length - 1] == '\n' || line[length - 1] == '\r'))
        line[--length] = '\0';

    Scr_AddString(line);
}

void GScr_WriteLine()
{
    if (Scr_GetNumParam() != 2)
    {
        DbgPrint("[codxe][IW5][GSC] fprintln expects 2 parameters\n");
        Scr_ErrorInternal();
        return;
    }

    const int handle = Scr_GetInt(0);
    if (handle < 1 || handle > MAX_SCRIPT_FILE_HANDLES || !script_file_handles[handle - 1].file)
    {
        DbgPrint("[codxe][IW5][GSC] fprintln received invalid handle %d\n", handle);
        Scr_AddInt(0);
        return;
    }

    const char *contents = Scr_GetString(1);
    Scr_AddInt(fprintf(script_file_handles[handle - 1].file, "%s\n", contents) >= 0);
}
} // namespace

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
Detour Scr_GetFunction_Detour;

unsigned int Scr_GetFunction_Hook(const char **pName, int *type)
{
    const unsigned int result = Scr_GetFunction_Detour.GetOriginal<decltype(Scr_GetFunction)>()(pName, type);

    if (!pName)
    {
        CloseAllScriptFiles();
        ResetBotState();
        // Retail registers addtestclient (0xEE), but its implementation is empty.
        Scr_RegisterFunction(reinterpret_cast<int>(GScr_AddTestClient), 0, 0xEE);
        Scr_RegisterFunction(reinterpret_cast<int>(GScr_OpenFile), 0, GSC_TOKEN_OPENFILE);
        Scr_RegisterFunction(reinterpret_cast<int>(GScr_CloseFile), 0, GSC_TOKEN_CLOSEFILE);
        Scr_RegisterFunction(reinterpret_cast<int>(GScr_WriteLine), 0, GSC_TOKEN_FPRINTLN);
        Scr_RegisterFunction(reinterpret_cast<int>(GScr_ReadLine), 0, GSC_TOKEN_FREADLN);
    }

    return result;
}

namespace
{
static const gsc::Entry<BuiltinMethod> methods[] = {
    {"setclientflags", PlayerCmd_SetClientFlags, BUILTIN_ANY},
    {"getclientflags", PlayerCmd_GetClientFlags, BUILTIN_ANY},
    {"setentityflags", PlayerCmd_SetEntityFlags, BUILTIN_ANY},
    {"getentityflags", PlayerCmd_GetEntityFlags, BUILTIN_ANY},
    {"disablebrushcollisionatorigin", DisableBrushCollisionAtOrigin, BUILTIN_ANY},
    {"botaction", PlayerCmd_BotAction, BUILTIN_ANY},
    {"botstop", PlayerCmd_BotStop, BUILTIN_ANY},
    {"botmovement", PlayerCmd_BotMovement, BUILTIN_ANY},
    {"botmeleeparams", PlayerCmd_BotMeleeParams, BUILTIN_ANY},
    {"botremoteangles", PlayerCmd_BotRemoteAngles, BUILTIN_ANY},
    {"botangles", PlayerCmd_BotAngles, BUILTIN_ANY},
};
} // namespace

void PlayerCmd_GetViewmodel_Hook(scr_entref_t entref)
{
    if (Scr_GetNumParam() > 0)
    {
        const char *selector = Scr_GetString(0);
        const gsc::Entry<BuiltinMethod> *method = gsc::Find(selector, methods);
        if (method)
        {
            method->actionFunc(entref);
            return;
        }
    }

    PlayerCmd_GetViewmodel_Detour.GetOriginal<PlayerCmd_GetViewmodel_t>()(entref);
}

GSC::GSC()
{
    Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
    Scr_GetFunction_Detour.Install();

    PlayerCmd_GetViewmodel_Detour = Detour(PlayerCmd_GetViewmodel, PlayerCmd_GetViewmodel_Hook);
    PlayerCmd_GetViewmodel_Detour.Install();
}

GSC::~GSC()
{
    CloseAllScriptFiles();
    PlayerCmd_GetViewmodel_Detour.Remove();
    Scr_GetFunction_Detour.Remove();
}
} // namespace mp
} // namespace iw5
