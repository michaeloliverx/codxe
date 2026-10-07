#include "pch.h"
#include "gsc.h"
#include "common/gsc_registry.h"
#include "clipmap.h"
#include "sv_bots.h"

namespace iw4
{
namespace mp_tu6
{
std::unordered_map<const char*, const char*> GSC::ReplacedFunctions;

static FILE *open_script_io_file_handle;
const int MAX_SCRIPT_STRING_BYTES = 65535;

void GScr_CbufAddText();
static void GScr_PrintConsole();
static void GScr_FileWrite();
static void GScr_FileRead();
static void GScr_FileExists();
static void GScr_OpenFile();
static void GScr_ReadStream();
static void GScr_CloseFile();

namespace
{
static const gsc::Entry<BuiltinFunction> functions[] = {
    {"exec", GScr_CbufAddText, BUILTIN_ANY},
    {"printconsole", GScr_PrintConsole, BUILTIN_ANY},
    {"filewrite", GScr_FileWrite, BUILTIN_ANY},
    {"fileread", GScr_FileRead, BUILTIN_ANY},
    {"fileexists", GScr_FileExists, BUILTIN_ANY},
    {"openfile", GScr_OpenFile, BUILTIN_ANY},
    {"readstream", GScr_ReadStream, BUILTIN_ANY},
    {"closefile", GScr_CloseFile, BUILTIN_ANY},
    {"addtestclient", GScr_AddTestClient, BUILTIN_ANY},
    {"replacefunc", GSC::ReplaceFunc, BUILTIN_ANY},
};

static const gsc::Entry<BuiltinMethod> methods[] = {
    {"disableplayercliponintersectingbrushes", ::DisablePlayerClipOnIntersectingBrushes, BUILTIN_ANY},
    {"botaction", PlayerCmd_BotAction, BUILTIN_ANY},
    {"botstop", PlayerCmd_BotStop, BUILTIN_ANY},
    {"botmovement", PlayerCmd_BotMovement, BUILTIN_ANY},
    {"botmeleeparams", PlayerCmd_BotMeleeParams, BUILTIN_ANY},
    {"botremoteangles", PlayerCmd_BotRemoteAngles, BUILTIN_ANY},
    {"botangles", PlayerCmd_BotAngles, BUILTIN_ANY},
};
} // namespace

Detour Scr_GetFunction_Detour;

BuiltinFunction Scr_GetFunction_Hook(const char **pName, scr_builtin_type_t *type)
{
    if (pName)
    {
        const gsc::Entry<BuiltinFunction> *function = gsc::Find(*pName, functions);
        if (function)
        {
            *type = static_cast<scr_builtin_type_t>(function->type);
            return function->actionFunc;
        }
    }
    else
    {
        for (size_t i = 0; i < gsc::Size(functions); ++i)
        {
            Scr_RegisterFunction(reinterpret_cast<int>(functions[i].actionFunc), functions[i].actionString);
        }
    }

    return Scr_GetFunction_Detour.GetOriginal<decltype(Scr_GetFunction)>()(pName, type);
}

Detour Scr_GetMethod_Detour;

BuiltinMethod Scr_GetMethod_Hook(const char **pName, scr_builtin_type_t *type)
{
    if (pName)
    {
        const gsc::Entry<BuiltinMethod> *method = gsc::Find(*pName, methods);
        if (method)
        {
            *type = static_cast<scr_builtin_type_t>(method->type);
            return method->actionFunc;
        }
    }
    else
    {
        for (size_t i = 0; i < gsc::Size(methods); ++i)
        {
            Scr_RegisterFunction(reinterpret_cast<int>(methods[i].actionFunc), methods[i].actionString);
        }
    }

    return Scr_GetMethod_Detour.GetOriginal<decltype(Scr_GetMethod)>()(pName, type);
}

Detour GSC::VM_Execute_Detour;

const char* GSC::GetCodePosForParam(int index)
{
    const int numParams = static_cast<int>(Scr_GetNumParam());

    if (index < 0 || index >= numParams)
        return nullptr;

    const VariableValue* value = &scrVmPub.top[-index];

    if (value->type != VAR_FUNCTION)
        return nullptr;

    return value->u.codePosValue;
}

const char* GSC::GetReplacedPos(const char* pos)
{
    if (!pos || ReplacedFunctions.empty())
        return pos;

    const auto it = ReplacedFunctions.find(pos);
    return it != ReplacedFunctions.end() ? it->second : pos;
}

extern "C" const char* IW4_GetReplacedPos(const char* pos)
{
    return GSC::GetReplacedPos(pos);
}

__declspec(naked) void GSC::VM_Execute_Hook()
{
    __asm
    {
        mr      r3, r11
        bl      IW4_GetReplacedPos
        mr      r11, r3

        lbz     r10, 0(r11)
        addi    r11, r11, 1
        stw     r11, 0x2028(r31)
        cmplwi  cr6, r10, 0x8B

        lis     r12, 0x822A
        ori     r12, r12, 0xEB20
        mtctr   r12
        bctr
    }
}

void GSC::ReplaceFunc()
{
    if (Scr_GetNumParam() != 2)
    {
        Scr_Error("replacefunc: expected two function parameters");
        return;
    }

    const char* what = GetCodePosForParam(0);
    const char* with = GetCodePosForParam(1);

    if (!what)
    {
        Scr_Error("replacefunc: first parameter must be a function");
        return;
    }

    if (!with)
    {
        Scr_Error("replacefunc: second parameter must be a function");
        return;
    }

    ReplacedFunctions[what] = with;
}

void GScr_CbufAddText()
{
    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: exec(<string>)\n");

    // VM strings are null-terminated, so no need to manually terminate
    // the string here.
    const char *text = Scr_GetString(0);
    Cbuf_AddText(0, text);
}

static void GScr_PrintConsole()
{
    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: printconsole(<string>)");

    const char *text = Scr_GetString(0);
    Com_Printf(0, "%s", text);
}

static void GScr_FileWrite()
{
    if (Scr_GetNumParam() != 3)
        Scr_Error("Usage: filewrite(<file>, <contents>, <mode>)");

    const char *filename = Scr_GetString(0);
    const char *contents = Scr_GetString(1);
    const char *mode = Scr_GetString(2);

    const char *file_mode = nullptr;
    if (!_stricmp(mode, "write"))
        file_mode = "wb";
    else if (!_stricmp(mode, "append"))
        file_mode = "ab";
    else
        Scr_ParamError(2, "filewrite: mode must be \"write\" or \"append\"");

    const std::string path = Config::ResolveModPath(filename);
    filesystem::CreateParentDirectories(path.c_str());

    FILE *file = fopen(path.c_str(), file_mode);
    if (!file)
    {
        Scr_AddInt(0);
        return;
    }

    const bool ok = fwrite(contents, 1, strlen(contents), file) == strlen(contents);
    fclose(file);

    Scr_AddInt(ok ? 1 : 0);
}

static void GScr_FileRead()
{
    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: fileread(<file>)");

    const std::string path = Config::ResolveModPath(Scr_GetString(0));
    FILE *file = fopen(path.c_str(), "rb");
    if (!file)
    {
        Scr_AddUndefined();
        return;
    }

    fseek(file, 0, SEEK_END);
    const long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size < 0)
    {
        fclose(file);
        Scr_AddUndefined();
        return;
    }

    const size_t file_size_bytes = static_cast<size_t>(file_size);
    const size_t bytes_to_read = file_size_bytes < MAX_SCRIPT_STRING_BYTES ? file_size_bytes : MAX_SCRIPT_STRING_BYTES;
    std::vector<char> buffer(bytes_to_read + 1);

    const size_t bytes_read = fread(buffer.data(), 1, bytes_to_read, file);
    fclose(file);

    if (bytes_read != bytes_to_read)
    {
        Scr_AddUndefined();
        return;
    }

    buffer[bytes_read] = '\0';
    Scr_AddString(buffer.data());
}

static void GScr_FileExists()
{
    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: fileexists(<file>)");

    const std::string path = Config::ResolveModPath(Scr_GetString(0));
    FILE *file = fopen(path.c_str(), "rb");
    if (!file)
    {
        Scr_AddInt(0);
        return;
    }

    fclose(file);
    Scr_AddInt(1);
}

static void GScr_OpenFile()
{
    if (Scr_GetNumParam() != 2)
        Scr_Error("Usage: openfile(<file>, <mode>)");

    const char *filename = Scr_GetString(0);
    const char *mode = Scr_GetString(1);

    if (_stricmp(mode, "read"))
    {
        Scr_AddInt(-1);
        return;
    }

    if (open_script_io_file_handle)
    {
        Scr_AddInt(-1);
        return;
    }

    const std::string path = Config::ResolveModPath(filename);
    open_script_io_file_handle = fopen(path.c_str(), "r");
    if (!open_script_io_file_handle)
    {
        Scr_AddInt(-1);
        return;
    }

    Scr_AddInt(1);
}

static void GScr_ReadStream()
{
    if (Scr_GetNumParam() != 0)
        Scr_Error("Usage: readstream()");

    if (!open_script_io_file_handle)
    {
        Scr_AddUndefined();
        return;
    }

    char line[4096];
    if (!fgets(line, sizeof(line), open_script_io_file_handle))
    {
        Scr_AddUndefined();
        return;
    }

    Scr_AddString(line);
}

static void CloseScriptIOFile()
{
    if (open_script_io_file_handle)
    {
        fclose(open_script_io_file_handle);
        open_script_io_file_handle = nullptr;
    }
}

void GSC::OnVMShutdown()
{
    ReplacedFunctions.clear();
    CloseScriptIOFile();
}

static void GScr_CloseFile()
{
    if (Scr_GetNumParam() != 0)
        Scr_Error("Usage: closefile()");

    if (!open_script_io_file_handle)
    {
        Scr_AddInt(-1);
        return;
    }

    const int result = fclose(open_script_io_file_handle);
    open_script_io_file_handle = nullptr;
    Scr_AddInt(result);
}

GSC::GSC()
{
    Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
    Scr_GetFunction_Detour.Install();

    Scr_GetMethod_Detour = Detour(Scr_GetMethod, Scr_GetMethod_Hook);
    Scr_GetMethod_Detour.Install();

    // Intercept TU6 VM_Execute's opcode fetch to redirect registered function code positions, then resume at
    // 0x822AEB20.
    VM_Execute_Detour = Detour(reinterpret_cast<void*>(0x822AEB10), reinterpret_cast<const void*>(VM_Execute_Hook));
    VM_Execute_Detour.Install();
}

GSC::~GSC()
{
    CloseScriptIOFile();

    Scr_GetFunction_Detour.Remove();

    Scr_GetMethod_Detour.Remove();

    VM_Execute_Detour.Remove();

    ReplacedFunctions.clear();
}
} // namespace mp_tu6
} // namespace iw4
