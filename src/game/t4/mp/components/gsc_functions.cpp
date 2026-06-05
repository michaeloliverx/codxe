#include "pch.h"
#include "gsc_functions.h"

namespace t4
{
namespace mp
{

#define MAX_SCRIPT_FILEHANDLES 8

struct ScriptFileHandle_t
{
    FILE *fh;
    char filename[MAX_PATH];
};

static ScriptFileHandle_t s_scriptFiles[MAX_SCRIPT_FILEHANDLES];

static std::string BuildScriptFilePath(const char *filename)
{
    if ((filename[0] && filename[1] == ':') || strncmp(filename, "game:\\", 6) == 0)
        return filename;

    std::string base = Config::GetModBasePath();
    if (base.empty())
        return filename;

    std::string rel(filename);
    for (size_t i = 0; i < rel.size(); ++i)
    {
        if (rel[i] == '/')
            rel[i] = '\\';
    }

    return base + "\\" + rel;
}

static void CloseAllScriptFiles()
{
    for (int i = 0; i < MAX_SCRIPT_FILEHANDLES; ++i)
    {
        if (s_scriptFiles[i].fh)
        {
            fclose(s_scriptFiles[i].fh);
            memset(&s_scriptFiles[i], 0, sizeof(ScriptFileHandle_t));
        }
    }
}

static void GScr_FS_TestFile()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("Usage: fs_testfile(<filename>)", SCRIPTINSTANCE_SERVER);

    const char *filename = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    std::string fullpath = BuildScriptFilePath(filename);
    FILE *f = fopen(fullpath.c_str(), "r");
    if (f)
    {
        fclose(f);
        Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
    }
}

static void GScr_FS_FOpen()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 2)
        Scr_Error("Usage: fs_fopen(<filename>, <mode>)", SCRIPTINSTANCE_SERVER);

    const char *filename = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    const char *mode_str = Scr_GetString(1, SCRIPTINSTANCE_SERVER);
    const char *fmode;

    if (!_stricmp(mode_str, "read"))
        fmode = "rt";
    else if (!_stricmp(mode_str, "write"))
        fmode = "wt";
    else if (!_stricmp(mode_str, "append"))
        fmode = "at";
    else
    {
        Scr_Error("fs_fopen: invalid mode. Valid modes are: read, write, append", SCRIPTINSTANCE_SERVER);
        return;
    }

    std::string fullpath = BuildScriptFilePath(filename);

    if (fmode[0] == 'w' || fmode[0] == 'a')
    {
        char dirpath[256];
        strncpy(dirpath, fullpath.c_str(), sizeof(dirpath) - 1);
        dirpath[sizeof(dirpath) - 1] = '\0';
        char *last_slash = strrchr(dirpath, '\\');
        if (last_slash)
        {
            *last_slash = '\0';
            filesystem::create_nested_dirs(dirpath);
        }
    }

    for (int i = 0; i < MAX_SCRIPT_FILEHANDLES; ++i)
    {
        if (!s_scriptFiles[i].fh)
        {
            s_scriptFiles[i].fh = fopen(fullpath.c_str(), fmode);
            if (!s_scriptFiles[i].fh)
            {
                Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
                return;
            }

            strncpy(s_scriptFiles[i].filename, filename, sizeof(s_scriptFiles[i].filename) - 1);
            Scr_AddInt(i + 1, SCRIPTINSTANCE_SERVER);
            return;
        }
    }

    Scr_Error("fs_fopen: exceeded maximum open file handles", SCRIPTINSTANCE_SERVER);
}

static void GScr_FS_FClose()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("Usage: fs_fclose(<filehandle>)", SCRIPTINSTANCE_SERVER);

    int fh = Scr_GetInt(0, SCRIPTINSTANCE_SERVER, 0, -1);
    if (fh < 1 || fh > MAX_SCRIPT_FILEHANDLES)
        Scr_Error("fs_fclose: invalid filehandle", SCRIPTINSTANCE_SERVER);

    ScriptFileHandle_t &slot = s_scriptFiles[fh - 1];
    if (slot.fh)
    {
        fclose(slot.fh);
        memset(&slot, 0, sizeof(ScriptFileHandle_t));
    }
}

static void GScr_FS_ReadLine()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("Usage: fs_readline(<filehandle>)", SCRIPTINSTANCE_SERVER);

    if (!Scr_AddString || !Scr_AddUndefined)
        Scr_Error("fs_readline: Scr_AddString/Scr_AddUndefined addresses are not set", SCRIPTINSTANCE_SERVER);

    int fh = Scr_GetInt(0, SCRIPTINSTANCE_SERVER, 0, -1);
    if (fh < 1 || fh > MAX_SCRIPT_FILEHANDLES)
        Scr_Error("fs_readline: invalid filehandle", SCRIPTINSTANCE_SERVER);

    ScriptFileHandle_t &slot = s_scriptFiles[fh - 1];
    if (!slot.fh)
        Scr_Error("fs_readline: filehandle is not open", SCRIPTINSTANCE_SERVER);

    char buffer[8192];
    if (!fgets(buffer, sizeof(buffer), slot.fh))
    {
        Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
        return;
    }

    int len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';

    Scr_AddString(buffer, SCRIPTINSTANCE_SERVER);
}

static void GScr_FS_WriteLine()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 2)
        Scr_Error("Usage: fs_writeline(<filehandle>, <data>)", SCRIPTINSTANCE_SERVER);

    int fh = Scr_GetInt(0, SCRIPTINSTANCE_SERVER, 0, -1);
    if (fh < 1 || fh > MAX_SCRIPT_FILEHANDLES)
        Scr_Error("fs_writeline: invalid filehandle", SCRIPTINSTANCE_SERVER);

    ScriptFileHandle_t &slot = s_scriptFiles[fh - 1];
    if (!slot.fh)
        Scr_Error("fs_writeline: filehandle is not open", SCRIPTINSTANCE_SERVER);

    const char *data = Scr_GetString(1, SCRIPTINSTANCE_SERVER);
    Scr_AddInt(fprintf(slot.fh, "%s\n", data) >= 0, SCRIPTINSTANCE_SERVER);
}

static void GScr_CmdExec()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("Usage: cmdexec(<string>)", SCRIPTINSTANCE_SERVER);

    const char *text = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    Cbuf_AddText(0, text);
}

/**
 * Checks if a 3D point is contained within an axis-aligned bounding box
 */
bool IsPointInBounds(const float mins[3], const float maxs[3], const float point[3])
{
    return (point[0] >= mins[0] && point[0] <= maxs[0]) && (point[1] >= mins[1] && point[1] <= maxs[1]) &&
           (point[2] >= mins[2] && point[2] <= maxs[2]);
}

void GSCrGetPlayerclipBrushesContainingPoint()
{
    float point[3] = {0};
    Scr_GetVector(0, point, SCRIPTINSTANCE_SERVER, -1);

    std::vector<int> brushIndices;
    for (int i = 0; i < cm->numBrushes; ++i)
    {
        auto &brush = cm->brushes[i];
        if (brush.contents & 0x10000 /* CONTENTS_PLAYERCLIP */ && IsPointInBounds(brush.mins, brush.maxs, point))
            brushIndices.push_back(i);
    }

    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    for (size_t i = 0; i < brushIndices.size(); ++i)
    {
        Scr_AddInt(brushIndices[i], SCRIPTINSTANCE_SERVER);
        Scr_AddArray(SCRIPTINSTANCE_SERVER);
    }
}

static struct
{
    const char *name;
    BuiltinFunction handler;
} gsc_functions[] = {
    {"getplayerclipbrushescontainingpoint", GSCrGetPlayerclipBrushesContainingPoint},
    {"fs_testfile", GScr_FS_TestFile},
    {"fs_fopen", GScr_FS_FOpen},
    {"fs_fclose", GScr_FS_FClose},
    {"fs_readline", GScr_FS_ReadLine},
    {"fs_writeline", GScr_FS_WriteLine},
    {"cmdexec", GScr_CmdExec},
    {nullptr, nullptr} // Terminator
};

Detour Scr_GetFunction_Detour;

BuiltinFunction Scr_GetFunction_Hook(const char **pName, int *type)
{
    if (pName != nullptr)
    {
        const auto *func = gsc_functions;
        for (; func->name != nullptr; ++func)
        {
            if (_stricmp(*pName, func->name) == 0)
                return func->handler;
        }
    }
    return Scr_GetFunction_Detour.GetOriginal<decltype(&Scr_GetFunction_Hook)>()(pName, type);
}

GSCFunctions::GSCFunctions()
{
    Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
    Scr_GetFunction_Detour.Install();
}

GSCFunctions::~GSCFunctions()
{
    Scr_GetFunction_Detour.Remove();
    CloseAllScriptFiles();
}
} // namespace mp
} // namespace t4
