#include "pch.h"
#include "gsc_functions.h"
#include "g_scr_main.h"
#include "events.h"

namespace iw3
{
namespace mp
{

#define MAX_SCRIPT_FILEHANDLES 8

struct ScriptFileHandle_t
{
    FILE *fh;
    char filename[256];
};

static ScriptFileHandle_t s_scriptFiles[MAX_SCRIPT_FILEHANDLES];

static std::string BuildScriptFilePath(const char *filename)
{
    // If already absolute, use as-is
    if ((filename[0] && filename[1] == ':') || strncmp(filename, "game:\\", 6) == 0)
        return filename;

    std::string base = Config::GetModBasePath();
    if (base.empty())
    {
        return filename;
    }

    // Normalize forward slashes to backslashes for Xbox 360
    std::string rel(filename);
    for (size_t i = 0; i < rel.size(); ++i)
        if (rel[i] == '/')
            rel[i] = '\\';

    std::string result = base + "\\" + rel;
    return result;
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
    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: fs_testfile(<filename>)");

    const char *filename = Scr_GetString(0);
    std::string fullpath = BuildScriptFilePath(filename);
    FILE *f = fopen(fullpath.c_str(), "r");
    if (f)
    {
        fclose(f);
        Scr_AddInt(1);
    }
    else
    {
        Scr_AddInt(0);
    }
}

static void GScr_FS_FOpen()
{
    if (Scr_GetNumParam() != 2)
        Scr_Error("Usage: fs_fopen(<filename>, <mode>)");

    const char *filename = Scr_GetString(0);
    const char *mode_str = Scr_GetString(1);
    const char *fmode;

    if (!stricmp(mode_str, "read"))
        fmode = "rt";
    else if (!stricmp(mode_str, "write"))
        fmode = "wt";
    else if (!stricmp(mode_str, "append"))
        fmode = "at";
    else
    {
        Scr_Error("fs_fopen: invalid mode. Valid modes are: read, write, append");
        return;
    }

    std::string fullpath = BuildScriptFilePath(filename);

    // Create parent directories for write/append modes
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
                Scr_AddInt(0);
                return;
            }
            strncpy(s_scriptFiles[i].filename, filename, sizeof(s_scriptFiles[i].filename) - 1);
            Scr_AddInt(i + 1);
            return;
        }
    }

    Scr_Error("fs_fopen: exceeded maximum open file handles");
}

static void GScr_FS_FClose()
{
    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: fs_fclose(<filehandle>)");

    int fh = Scr_GetInt(0);
    if (fh < 1 || fh > MAX_SCRIPT_FILEHANDLES)
        Scr_Error("fs_fclose: invalid filehandle");

    ScriptFileHandle_t &slot = s_scriptFiles[fh - 1];
    if (slot.fh)
    {
        fclose(slot.fh);
        memset(&slot, 0, sizeof(ScriptFileHandle_t));
    }
}

static void GScr_FS_ReadLine()
{
    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: fs_readline(<filehandle>)");

    int fh = Scr_GetInt(0);
    if (fh < 1 || fh > MAX_SCRIPT_FILEHANDLES)
        Scr_Error("fs_readline: invalid filehandle");

    ScriptFileHandle_t &slot = s_scriptFiles[fh - 1];
    if (!slot.fh)
        Scr_Error("fs_readline: filehandle is not open");

    char buffer[8192];
    if (!fgets(buffer, sizeof(buffer), slot.fh))
    {
        Scr_AddUndefined();
        return;
    }

    int len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';

    Scr_AddString(buffer);
}

static void GScr_FS_WriteLine()
{
    if (Scr_GetNumParam() != 2)
        Scr_Error("Usage: fs_writeline(<filehandle>, <data>)");

    int fh = Scr_GetInt(0);
    if (fh < 1 || fh > MAX_SCRIPT_FILEHANDLES)
        Scr_Error("fs_writeline: invalid filehandle");

    ScriptFileHandle_t &slot = s_scriptFiles[fh - 1];
    if (!slot.fh)
        Scr_Error("fs_writeline: filehandle is not open");

    const char *data = Scr_GetString(1);
    if (fprintf(slot.fh, "%s\n", data) < 0)
    {
        Scr_AddInt(0);
        return;
    }

    Scr_AddInt(1);
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
    Scr_GetVector(0, point);

    std::vector<int> brushIndices;
    for (int i = 0; i < cm->numBrushes; ++i)
    {
        auto &brush = cm->brushes[i];
        if (brush.contents & CONTENTS_PLAYERCLIP && IsPointInBounds(brush.mins, brush.maxs, point))
            brushIndices.push_back(i);
    }

    Scr_MakeArray();
    for (size_t i = 0; i < brushIndices.size(); ++i)
    {
        Scr_AddInt(brushIndices[i]);
        Scr_AddArray();
    }
}

void GScr_CbufAddText()
{
    if (Scr_GetNumParam() != 1)
    {
        Scr_Error("Usage: exec(<string>)\n");
    }
    // VM strings are null-terminated, so no need to manually terminate
    // the string here.
    const char *text = Scr_GetString(0);
    Cbuf_AddText(0, text);
}

void Scr_IsArray_f()
{
    if (Scr_GetNumParam() != 1)
    {
        Scr_Error("usage: isArray(<variable>)");
        return;
    }

    Scr_AddInt(Scr_GetType(0) == 1);
}

void GScr_Float()
{
    if (Scr_GetNumParam() != 1)
    {
        Scr_Error("Usage: floatVal = float(<float, int, bool or string>);");
        return;
    }

    int varType = Scr_GetType(0);
    if (varType == VAR_FLOAT)
        Scr_AddFloat(Scr_GetFloat(0));
    else if (varType == VAR_INTEGER)
        Scr_AddFloat(1.0f * Scr_GetInt(0));
    else if (varType == VAR_STRING)
    {
        const char *strFloat = Scr_GetString(0);
        double result = 0.0;
        if (isdigit(strFloat[0]) || (strFloat[0] == '-' && isdigit(strFloat[1])))
            result = atof(strFloat);
        Scr_AddFloat((float)result);
    }
    else
        Scr_ParamError(0, va("cannot cast %s to float", var_typename[varType]));
}

gsc_functions::gsc_functions()
{
    Scr_AddFunction("exec", GScr_CbufAddText, 0);
    Scr_AddFunction("getplayerclipbrushescontainingpoint", GSCrGetPlayerclipBrushesContainingPoint, 0);
    Scr_AddFunction("fs_testfile", GScr_FS_TestFile, 0);
    Scr_AddFunction("fs_fopen", GScr_FS_FOpen, 0);
    Scr_AddFunction("fs_fclose", GScr_FS_FClose, 0);
    Scr_AddFunction("fs_readline", GScr_FS_ReadLine, 0);
    Scr_AddFunction("fs_writeline", GScr_FS_WriteLine, 0);
    Scr_AddFunction("isarray", Scr_IsArray_f, false);
    Scr_AddFunction("float", GScr_Float, 0);

    Events::OnVMShutdown(CloseAllScriptFiles);
}

gsc_functions::~gsc_functions()
{
    CloseAllScriptFiles();
}
} // namespace mp
} // namespace iw3
