#include "pch.h"
#include "g_scr_main.h"

namespace iw3
{
namespace mp
{
namespace
{
const size_t MAX_CUSTOM_SCRIPT_FUNCTIONS = 128;
const size_t MAX_CUSTOM_SCRIPT_METHODS = 128;

BuiltinFunctionDef scr_functions[MAX_CUSTOM_SCRIPT_FUNCTIONS];
BuiltinMethodDef scr_methods[MAX_CUSTOM_SCRIPT_METHODS];
size_t scr_function_count = 0;
size_t scr_method_count = 0;
} // namespace

void Scr_ClearBuiltins()
{
    ZeroMemory(scr_functions, sizeof(scr_functions));
    ZeroMemory(scr_methods, sizeof(scr_methods));
    scr_function_count = 0;
    scr_method_count = 0;
}

void Scr_AddFunction(const char *name, BuiltinFunction func, int type)
{
    if (scr_function_count >= MAX_CUSTOM_SCRIPT_FUNCTIONS)
    {
        DbgPrint("[codxe][g_scr_main] Too many custom script functions; ignoring '%s'.\n", name);
        return;
    }

    BuiltinFunctionDef &newFunc = scr_functions[scr_function_count++];
    newFunc.actionString = name;
    newFunc.actionFunc = func;
    newFunc.type = type;
}

void Scr_AddMethod(const char *name, BuiltinMethod func, int type)
{
    if (scr_method_count >= MAX_CUSTOM_SCRIPT_METHODS)
    {
        DbgPrint("[codxe][g_scr_main] Too many custom script methods; ignoring '%s'.\n", name);
        return;
    }

    BuiltinMethodDef &newMethod = scr_methods[scr_method_count++];
    newMethod.actionString = name;
    newMethod.actionFunc = func;
    newMethod.type = type;
}

Detour Scr_GetFunction_Detour;

BuiltinFunction Scr_GetFunction_Hook(const char **pName, int *type)
{
    if (pName != nullptr)
    {
        for (size_t i = 0; i < scr_function_count; ++i)
        {
            if (std::strcmp(*pName, scr_functions[i].actionString) == 0)
            {
                *type = scr_functions[i].type;
                return scr_functions[i].actionFunc;
            }
        }
    }

    return Scr_GetFunction_Detour.GetOriginal<decltype(Scr_GetFunction)>()(pName, type);
}

Detour Scr_GetMethod_Detour;

BuiltinMethod Scr_GetMethod_Hook(const char **pName, int *type)
{
    if (pName != nullptr)
    {
        for (size_t i = 0; i < scr_method_count; ++i)
        {
            if (std::strcmp(*pName, scr_methods[i].actionString) == 0)
            {
                *type = scr_methods[i].type;
                return scr_methods[i].actionFunc;
            }
        }
    }

    return Scr_GetMethod_Detour.GetOriginal<decltype(Scr_GetMethod)>()(pName, type);
}

g_scr_main::g_scr_main()
{
    Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
    Scr_GetFunction_Detour.Install();

    Scr_GetMethod_Detour = Detour(Scr_GetMethod, Scr_GetMethod_Hook);
    Scr_GetMethod_Detour.Install();
}

g_scr_main::~g_scr_main()
{
    Scr_GetFunction_Detour.Remove();

    Scr_GetMethod_Detour.Remove();

    Scr_ClearBuiltins();
}
} // namespace mp
} // namespace iw3
