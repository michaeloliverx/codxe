#include "pch.h"
#include "gsc.h"
#include "common/gsc_registry.h"

namespace iw3
{
namespace sp
{
namespace
{
void GScr_CbufAddText()
{
    if (Scr_GetNumParam() != 1)
    {
        Scr_Error("Usage: exec(<string>)\n");
        return;
    }

    const char *text = Scr_GetString(0);
    Cbuf_AddText(0, text);
}

static const gsc::Entry<BuiltinFunction> functions[] = {
    {"exec", GScr_CbufAddText, BUILTIN_ANY},
};

Detour Scr_GetFunction_Detour;

BuiltinFunction Scr_GetFunction_Hook(const char **pName, int *type)
{
    if (pName)
    {
        const gsc::Entry<BuiltinFunction> *function = gsc::Find(*pName, functions);
        if (function)
        {
            *type = function->type;
            return function->actionFunc;
        }
    }

    return Scr_GetFunction_Detour.GetOriginal<decltype(Scr_GetFunction)>()(pName, type);
}
} // namespace

GSC::GSC()
{
    Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
    Scr_GetFunction_Detour.Install();
}
} // namespace sp
} // namespace iw3
