#include "pch.h"
#include "g_scr_main.h"
#include "common/gsc_registry.h"
#include "gsc_functions.h"
#include "gsc_hud_elem.h"
#include "gsc_methods.h"
#include "sv_bots.h"

namespace iw3
{
namespace mp
{
namespace
{
static const BuiltinFunctionDef functions[] = {
    {"exec", GScr_CbufAddText, 0},
    {"getplayerclipbrushescontainingpoint", GSCrGetPlayerclipBrushesContainingPoint, 0},
    {"fs_testfile", GScr_FS_TestFile, 0},
    {"fs_fopen", GScr_FS_FOpen, 0},
    {"fs_fclose", GScr_FS_FClose, 0},
    {"fs_readline", GScr_FS_ReadLine, 0},
    {"fs_writeline", GScr_FS_WriteLine, 0},
    {"isarray", Scr_IsArray_f, 0},
    {"float", GScr_Float, 0},
    {"precachestring", Scr_PrecacheString_Stub, 0},
    {"addtestclient", GScr_AddTestClient, 0},
};

static const BuiltinMethodDef methods[] = {
    {"settext", HECmd_SetText_Stub, 0},
    {"buttonpressed", PlayerCmd_ButtonPressed, 0}, // Host-only
    {"sprintbreathbuttonpressed", PlayerCmd_SprintButtonPressed, 0},
    {"leanleftbuttonpressed", PlayerCmd_LeanLeftButtonPressed, 0},
    {"leanrightbuttonpressed", PlayerCmd_LeanRightButtonPressed, 0},
    {"jumpbuttonpressed", PlayerCmd_JumpButtonPressed, 0},
    {"holdbreathbuttonpressed", PlayerCmd_HoldBreathButtonPressed, 0},
    {"nightvisionbuttonpressed", PlayerCmd_NightVisionButtonPressed, 0},
    {"forwardbuttonpressed", PlayerCmd_ForwardButtonPressed, 0},
    {"backbuttonpressed", PlayerCmd_BackButtonPressed, 0},
    {"leftbuttonpressed", PlayerCmd_LeftButtonPressed, 0},
    {"rightbuttonpressed", PlayerCmd_RightButtonPressed, 0},
    {"setvelocity", PlayerCmd_SetVelocity, 0},
    {"setstance", PlayerCmd_SetStance, 0},
    {"clonebrushmodeltoscriptmodel", GScr_CloneBrushModelToScriptModel, 0},
    {"setbrushmodel", GScr_SetBrushModel, 0},
    {"botmoveto", Scr_BotMoveTo, 0},
    {"botaction", Scr_BotAction, 0},
    {"botmirror", Scr_BotMirror, 0},
    {"botstop", Scr_BotStop, 0},
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
} // namespace mp
} // namespace iw3
