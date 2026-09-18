#include "pch.h"
#include "gsc_functions.h"
#include "common/gsc_registry.h"
#include "common/script_files.h"
#include "sv_bots.h"
#include <unordered_map>

namespace t4
{
namespace mp
{
static std::unordered_map<const char*, const char*> ReplacedFunctions;
static void CloseAllScriptFiles()
{
    script_files::CloseAll();
}

void GSCFunctions::OnVMShutdown()
{
    CloseAllScriptFiles();
}

static void GScr_FS_TestFile()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("Usage: fs_testfile(<filename>)", SCRIPTINSTANCE_SERVER);

    const char *filename = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    const std::string fullpath = Config::ResolveModPath(filename);
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

    const std::string fullpath = Config::ResolveModPath(filename);

    if (fmode[0] == 'w' || fmode[0] == 'a')
        filesystem::CreateParentDirectories(fullpath.c_str());

    const int handle = script_files::Open(fullpath.c_str(), fmode);
    if (handle == script_files::NO_FREE_HANDLES)
    {
        Scr_Error("fs_fopen: exceeded maximum open file handles", SCRIPTINSTANCE_SERVER);
        return;
    }

    Scr_AddInt(handle, SCRIPTINSTANCE_SERVER);
}

static void GScr_FS_FClose()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("Usage: fs_fclose(<filehandle>)", SCRIPTINSTANCE_SERVER);

    int fh = Scr_GetInt(0, SCRIPTINSTANCE_SERVER, 0, -1);
    if (!script_files::IsValidHandle(fh))
        Scr_Error("fs_fclose: invalid filehandle", SCRIPTINSTANCE_SERVER);

    if (script_files::Get(fh))
        script_files::Close(fh);
}

static void GScr_FS_ReadLine()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("Usage: fs_readline(<filehandle>)", SCRIPTINSTANCE_SERVER);

    if (!Scr_AddString || !Scr_AddUndefined)
        Scr_Error("fs_readline: Scr_AddString/Scr_AddUndefined addresses are not set", SCRIPTINSTANCE_SERVER);

    int fh = Scr_GetInt(0, SCRIPTINSTANCE_SERVER, 0, -1);
    if (!script_files::IsValidHandle(fh))
        Scr_Error("fs_readline: invalid filehandle", SCRIPTINSTANCE_SERVER);

    FILE *file = script_files::Get(fh);
    if (!file)
        Scr_Error("fs_readline: filehandle is not open", SCRIPTINSTANCE_SERVER);

    char buffer[8192];
    if (!fgets(buffer, sizeof(buffer), file))
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
    if (!script_files::IsValidHandle(fh))
        Scr_Error("fs_writeline: invalid filehandle", SCRIPTINSTANCE_SERVER);

    FILE *file = script_files::Get(fh);
    if (!file)
        Scr_Error("fs_writeline: filehandle is not open", SCRIPTINSTANCE_SERVER);

    const char *data = Scr_GetString(1, SCRIPTINSTANCE_SERVER);
    Scr_AddInt(fprintf(file, "%s\n", data) >= 0, SCRIPTINSTANCE_SERVER);
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

static const char* GetCodePosForParam(int index)
{
    const scriptInstance_t inst = SCRIPTINSTANCE_SERVER;
    const int numParams = static_cast<int>(Scr_GetNumParam(inst));

    if (index < 0 || index >= numParams)
        return nullptr;

    auto* top = *reinterpret_cast<VariableValue**>(0x85B11094 + (static_cast<int>(inst) * 0x14));

    if (!top)
        return nullptr;

    auto* value = top - index;

    if (value->type != VAR_FUNCTION)
        return nullptr;

    return value->u.codePosValue;
}

static const char* GSC_ResolveReplaceFunc(const char* pos)
{
    if (!pos)
        return nullptr;

    const auto it = ReplacedFunctions.find(pos);

    if (it == ReplacedFunctions.end())
        return nullptr;

    printf(
        "[ReplaceFunc HIT] %p -> %p | count=%u\n",
        pos,
        it->second,
        static_cast<unsigned int>(ReplacedFunctions.size())
    );

    return it->second;
}

extern "C" const char* T4_GSC_GetReplacedPos(const char* pos)
{
    const char* replacement = GSC_ResolveReplaceFunc(pos);

    if (replacement)
    {
        printf("[ReplaceFunc VM HIT] %p -> %p\n", pos, replacement);
        return replacement;
    }

    return pos;
}

static void GScr_ReplaceFunc()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 2)
    {
        Scr_Error(
            "replaceFunc: needs two parameters",
            SCRIPTINSTANCE_SERVER
        );
        return;
    }

    const char* what = GetCodePosForParam(0);
    const char* with = GetCodePosForParam(1);

    if (!what)
    {
        Scr_Error(
            "replaceFunc: first parameter is not a function",
            SCRIPTINSTANCE_SERVER
        );
        return;
    }

    if (!with)
    {
        Scr_Error(
            "replaceFunc: second parameter is not a function",
            SCRIPTINSTANCE_SERVER
        );
        return;
    }

    printf(
        "[ReplaceFunc REGISTER] %p -> %p | before=%u\n",
        what,
        with,
        static_cast<unsigned int>(ReplacedFunctions.size())
    );

    ReplacedFunctions[what] = with;

    printf(
        "[ReplaceFunc REGISTER] after=%u\n",
        static_cast<unsigned int>(ReplacedFunctions.size())
    );
}

Detour VMExecuteInternal_Detour;
extern "C" __declspec(naked) void VM_Execute_Hook()
{
    __asm
    {
        mr      r3, r10
        bl      T4_GSC_GetReplacedPos

        mr      r10, r3

        addi    r11, r10, 1
        lbz     r9, 0(r10)
        addi    r10, r27, -0x4068
        slwi    r21, r31, 2

        lis     r12, 0x8234
        ori     r12, r12, 0x6480
        mtctr   r12
        bctr
    }
}

void GSCFunctions::ClearReplacedFunctions()
{
    printf(
        "[ReplaceFunc CLEAR] before=%u\n",
        static_cast<unsigned int>(ReplacedFunctions.size())
    );

    for (auto it = ReplacedFunctions.begin(); it != ReplacedFunctions.end(); ++it)
    {
        printf(
            "[ReplaceFunc CLEAR] removing %p -> %p\n",
            it->first,
            it->second
        );
    }

    ReplacedFunctions.clear();

    printf(
        "[ReplaceFunc CLEAR] after=%u\n",
        static_cast<unsigned int>(ReplacedFunctions.size())
    );
}

static const gsc::Entry<BuiltinFunction> functions[] = {
    {"addtestclient", GScr_AddTestClient, BUILTIN_ANY},
    {"replaceFunc", GScr_ReplaceFunc, BUILTIN_ANY},
    {"getplayerclipbrushescontainingpoint", GSCrGetPlayerclipBrushesContainingPoint, BUILTIN_ANY},
    {"fs_testfile", GScr_FS_TestFile, BUILTIN_ANY},
    {"fs_fopen", GScr_FS_FOpen, BUILTIN_ANY},
    {"fs_fclose", GScr_FS_FClose, BUILTIN_ANY},
    {"fs_readline", GScr_FS_ReadLine, BUILTIN_ANY},
    {"fs_writeline", GScr_FS_WriteLine, BUILTIN_ANY},
    {"cmdexec", GScr_CmdExec, BUILTIN_ANY},
};

Detour Scr_GetFunction_Detour;

BuiltinFunction Scr_GetFunction_Hook(const char **pName, int *type)
{
    if (pName)
    {
        const gsc::Entry<BuiltinFunction> *function = gsc::Find(*pName, functions);
        if (function)
            return function->actionFunc;
    }
    return Scr_GetFunction_Detour.GetOriginal<decltype(&Scr_GetFunction_Hook)>()(pName, type);
}

GSCFunctions::GSCFunctions()
{
    Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
    Scr_GetFunction_Detour.Install();

    VMExecuteInternal_Detour = Detour(reinterpret_cast<void*>(0x82346470), reinterpret_cast<const void*>(VM_Execute_Hook));
    VMExecuteInternal_Detour.Install();
}

GSCFunctions::~GSCFunctions()
{
    Scr_GetFunction_Detour.Remove();
    VMExecuteInternal_Detour.Remove();
    CloseAllScriptFiles();
}
} // namespace mp
} // namespace t4
