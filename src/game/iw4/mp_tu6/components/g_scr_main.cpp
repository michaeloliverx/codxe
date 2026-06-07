#include "pch.h"
#include "g_scr_main.h"

namespace iw4
{
namespace mp_tu6
{
std::vector<BuiltinFunctionDef *> scr_functions;
std::vector<BuiltinMethodDef *> scr_methods;

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
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file)
    {
        Scr_AddUndefined();
        return;
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    Scr_AddString(contents.str().c_str());
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
}

g_scr_main::~g_scr_main()
{
    Scr_GetFunction_Detour.Remove();

    Scr_GetMethod_Detour.Remove();
}
} // namespace mp_tu6
} // namespace iw4
