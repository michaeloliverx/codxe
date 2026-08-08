#include "pch.h"
#include "g_scr_main.h"
#include "common/gsc_registry.h"

namespace iw4
{
namespace sp
{
void GScr_CbufAddText();

namespace
{
static const BuiltinFunctionDef functions[] = {
    {"exec", GScr_CbufAddText, 0},
};
} // namespace

Detour Scr_GetFunction_Detour;

BuiltinFunction Scr_GetFunction_Hook(const char **pName, int *type)
{
    if (pName)
    {
        const BuiltinFunctionDef *function = gsc::Find(*pName, functions);
        if (function)
        {
            *type = function->type;
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

BuiltinMethod Scr_GetMethod_Hook(const char **pName, int *type)
{
    return Scr_GetMethod_Detour.GetOriginal<decltype(Scr_GetMethod)>()(pName, type);
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
}
} // namespace sp
} // namespace iw4
