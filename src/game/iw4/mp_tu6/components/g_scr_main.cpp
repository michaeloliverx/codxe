#include "pch.h"
#include "g_scr_main.h"
#include "events.h"

namespace iw4
{
namespace mp_tu6
{
std::vector<BuiltinFunctionDef *> scr_functions;
std::vector<BuiltinMethodDef *> scr_methods;
static FILE *open_script_io_file_handle;
const int MAX_SCRIPT_STRING_BYTES = 65535;

void Scr_AddFunction(const char *name, BuiltinFunction func, scr_builtin_type_t type)
{
    BuiltinFunctionDef *newFunc = new BuiltinFunctionDef;
    newFunc->actionString = name;
    newFunc->actionFunc = func;
    newFunc->type = type;
    scr_functions.push_back(newFunc);
}

void Scr_AddMethod(const char *name, BuiltinMethod func, scr_builtin_type_t type)
{
    BuiltinMethodDef *newMethod = new BuiltinMethodDef;
    newMethod->actionString = name;
    newMethod->actionFunc = func;
    newMethod->type = type;
    scr_methods.push_back(newMethod);
}

Detour Scr_GetFunction_Detour;

BuiltinFunction Scr_GetFunction_Hook(const char **pName, scr_builtin_type_t *type)
{
    if (pName != nullptr)
    {
        for (size_t i = 0; i < scr_functions.size(); ++i)
        {
            if (std::strcmp(*pName, scr_functions[i]->actionString) == 0)
            {
                *type = scr_functions[i]->type;
                return scr_functions[i]->actionFunc;
            }
        }
    }
    else
    {
        for (size_t i = 0; i < scr_functions.size(); ++i)
        {
            Scr_RegisterFunction(reinterpret_cast<int>(scr_functions[i]->actionFunc), scr_functions[i]->actionString);
        }
    }

    return Scr_GetFunction_Detour.GetOriginal<decltype(Scr_GetFunction)>()(pName, type);
}

Detour Scr_GetMethod_Detour;

BuiltinMethod Scr_GetMethod_Hook(const char **pName, scr_builtin_type_t *type)
{
    if (pName != nullptr)
    {
        for (size_t i = 0; i < scr_methods.size(); ++i)
        {
            if (std::strcmp(*pName, scr_methods[i]->actionString) == 0)
            {
                *type = scr_methods[i]->type;
                return scr_methods[i]->actionFunc;
            }
        }
    }
    else
    {
        for (size_t i = 0; i < scr_methods.size(); ++i)
        {
            Scr_RegisterFunction(reinterpret_cast<int>(scr_methods[i]->actionFunc), scr_methods[i]->actionString);
        }
    }

    return Scr_GetMethod_Detour.GetOriginal<decltype(Scr_GetMethod)>()(pName, type);
}

static std::string BuildScriptFilePath(const char *filename)
{
    if ((filename[0] && filename[1] == ':') || strncmp(filename, "game:\\", 6) == 0)
        return filename;

    std::string rel(filename);
    for (size_t i = 0; i < rel.size(); ++i)
    {
        if (rel[i] == '/')
            rel[i] = '\\';
    }

    const std::string base = Config::GetModBasePath();
    if (base.empty())
        return rel;

    return base + "\\" + rel;
}

static void EnsureParentDirectory(const std::string &path)
{
    char dirpath[MAX_PATH];
    strncpy(dirpath, path.c_str(), sizeof(dirpath) - 1);
    dirpath[sizeof(dirpath) - 1] = '\0';

    char *last_slash = strrchr(dirpath, '\\');
    if (last_slash)
    {
        *last_slash = '\0';
        filesystem::create_nested_dirs(dirpath);
    }
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

    const std::string path = BuildScriptFilePath(filename);
    EnsureParentDirectory(path);

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

    const std::string path = BuildScriptFilePath(Scr_GetString(0));
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

    const std::string path = BuildScriptFilePath(Scr_GetString(0));
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

    const std::string path = BuildScriptFilePath(filename);
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

g_scr_main::g_scr_main()
{
    Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
    Scr_GetFunction_Detour.Install();

    Scr_GetMethod_Detour = Detour(Scr_GetMethod, Scr_GetMethod_Hook);
    Scr_GetMethod_Detour.Install();

    Scr_AddFunction("exec", GScr_CbufAddText, BUILTIN_ANY);
    Scr_AddFunction("printconsole", GScr_PrintConsole, BUILTIN_ANY);
    Scr_AddFunction("filewrite", GScr_FileWrite, BUILTIN_ANY);
    Scr_AddFunction("fileread", GScr_FileRead, BUILTIN_ANY);
    Scr_AddFunction("fileexists", GScr_FileExists, BUILTIN_ANY);
    Scr_AddFunction("openfile", GScr_OpenFile, BUILTIN_ANY);
    Scr_AddFunction("readstream", GScr_ReadStream, BUILTIN_ANY);
    Scr_AddFunction("closefile", GScr_CloseFile, BUILTIN_ANY);
    Events::OnVMShutdown(CloseScriptIOFile);
}

g_scr_main::~g_scr_main()
{
    CloseScriptIOFile();

    Scr_GetFunction_Detour.Remove();

    Scr_GetMethod_Detour.Remove();
}
} // namespace mp_tu6
} // namespace iw4
