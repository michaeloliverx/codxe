#include "pch.h"
#include "g_scr_main.h"
#include "common/gsc_registry.h"
#include "g_client_script_cmd.h"
#include "g_scr_mover.h"

namespace t4
{
namespace sp
{
namespace
{
static const BuiltinMethodDef methods[] = {
    {"jumpbuttonpressed", PlayerCmd_JumpButtonPressed, 0},
    {"secondaryoffhandbuttonpressed", PlayerCmd_secondaryOffhandButtonPressed, 0},
    {"sprintbuttonpressed", PlayerCmd_SprintButtonPressed, 0},
    {"moveforwardbuttonpressed", PlayerCmd_MoveForwardButtonPressed, 0},
    {"movebackbuttonpressed", PlayerCmd_MoveBackButtonPressed, 0},
    {"moveleftbuttonpressed", PlayerCmd_MoveLeftButtonPressed, 0},
    {"moverightbuttonpressed", PlayerCmd_MoveRightButtonPressed, 0},
    {"clonebrushmodeltoscriptmodel", ScriptEntCmd_CloneBrushModelToScriptModel, 0},
};
} // namespace

Detour Scr_GetFunction_Detour;

BuiltinFunction Scr_GetFunction_Hook(const char **pName, int *type)
{
    return Scr_GetFunction_Detour.GetOriginal<decltype(Scr_GetFunction)>()(pName, type);
}

Detour Scr_GetMethod_Detour;

BuiltinMethod Scr_GetMethod_Hook(const char **pName, int *type)
{
    if (pName)
    {
        const BuiltinMethodDef *method = gsc::Find(*pName, methods);
        if (method)
        {
            *type = method->type;
            return method->actionFunc;
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
}
} // namespace sp
} // namespace t4
