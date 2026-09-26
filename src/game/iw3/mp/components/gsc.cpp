#include "pch.h"
#include "gsc.h"
#include "common/gsc_registry.h"
#include "gsc_functions.h"
#include "gsc_methods.h"
#include "sv_bots.h"
#include <unordered_map>

namespace iw3
{
namespace mp
{
static std::unordered_map<const char*, const char*> ReplacedFunctions;
namespace
{
static const gsc::Entry<BuiltinFunction> functions[] = {
    {"exec", GScr_CbufAddText, BUILTIN_ANY},
    {"getplayerclipbrushescontainingpoint", GSCrGetPlayerclipBrushesContainingPoint, BUILTIN_ANY},
    {"fs_testfile", GScr_FS_TestFile, BUILTIN_ANY},
    {"fs_fopen", GScr_FS_FOpen, BUILTIN_ANY},
    {"fs_fclose", GScr_FS_FClose, BUILTIN_ANY},
    {"fs_readline", GScr_FS_ReadLine, BUILTIN_ANY},
    {"fs_writeline", GScr_FS_WriteLine, BUILTIN_ANY},
    {"isarray", Scr_IsArray_f, BUILTIN_ANY},
    {"float", GScr_Float, BUILTIN_ANY},
    {"precachestring", Scr_PrecacheString_Stub, BUILTIN_ANY},
    {"addtestclient", GScr_AddTestClient, BUILTIN_ANY},
    {"replacefunc", GScr_ReplaceFunc, BUILTIN_ANY},
};

static const gsc::Entry<BuiltinMethod> methods[] = {
    {"settext", HECmd_SetText_Stub, BUILTIN_ANY},
    {"buttonpressed", PlayerCmd_ButtonPressed, BUILTIN_ANY}, // Host-only
    {"sprintbreathbuttonpressed", PlayerCmd_SprintButtonPressed, BUILTIN_ANY},
    {"leanleftbuttonpressed", PlayerCmd_LeanLeftButtonPressed, BUILTIN_ANY},
    {"leanrightbuttonpressed", PlayerCmd_LeanRightButtonPressed, BUILTIN_ANY},
    {"jumpbuttonpressed", PlayerCmd_JumpButtonPressed, BUILTIN_ANY},
    {"holdbreathbuttonpressed", PlayerCmd_HoldBreathButtonPressed, BUILTIN_ANY},
    {"nightvisionbuttonpressed", PlayerCmd_NightVisionButtonPressed, BUILTIN_ANY},
    {"forwardbuttonpressed", PlayerCmd_ForwardButtonPressed, BUILTIN_ANY},
    {"backbuttonpressed", PlayerCmd_BackButtonPressed, BUILTIN_ANY},
    {"leftbuttonpressed", PlayerCmd_LeftButtonPressed, BUILTIN_ANY},
    {"rightbuttonpressed", PlayerCmd_RightButtonPressed, BUILTIN_ANY},
    {"setvelocity", PlayerCmd_SetVelocity, BUILTIN_ANY},
    {"setstance", PlayerCmd_SetStance, BUILTIN_ANY},
    {"clonebrushmodeltoscriptmodel", GScr_CloneBrushModelToScriptModel, BUILTIN_ANY},
    {"setbrushmodel", GScr_SetBrushModel, BUILTIN_ANY},
    {"botmoveto", Scr_BotMoveTo, BUILTIN_ANY},
    {"botaction", Scr_BotAction, BUILTIN_ANY},
    {"botmirror", Scr_BotMirror, BUILTIN_ANY},
    {"botstop", Scr_BotStop, BUILTIN_ANY},
};
} // namespace

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

Detour Scr_GetMethod_Detour;

BuiltinMethod Scr_GetMethod_Hook(const char **pName, int *type)
{
    if (pName)
    {
        const gsc::Entry<BuiltinMethod> *method = gsc::Find(*pName, methods);
        if (method)
        {
            *type = method->type;
            return method->actionFunc;
        }
    }

    return Scr_GetMethod_Detour.GetOriginal<decltype(Scr_GetMethod)>()(pName, type);
}

Detour VM_Execute_Detour;

const char* GetCodePosForParam(int index)
{
    const int numParams = static_cast<int>(Scr_GetNumParam());

    if (index < 0 || index >= numParams)
        return nullptr;

    const VariableValue *value = &scrVmPub.top[-index];

    if (value->type != VAR_FUNCTION)
        return nullptr;

    return value->u.codePosValue;
}

extern "C" const char* IW3_GSC_GetReplacedPos(const char* pos)
{
    if (!pos)
        return nullptr;

    const auto it = ReplacedFunctions.find(pos);
    return it != ReplacedFunctions.end() ? it->second : pos;
}

extern "C" __declspec(naked) void VM_Execute_Hook()
{
    __asm
    {
        mr      r3, r11
        bl      IW3_GSC_GetReplacedPos

        mr      r11, r3
        mr      r10, r3

        lbz     r11, 0(r11)
        addi    r10, r10, 1
        cmplwi  cr6, r11, 0x86
        stw     r11, 0x400(r17)

        lis     r12, 0x8221
        ori     r12, r12, 0x2E40
        mtctr   r12
        bctr
    }
}

void GScr_ReplaceFunc()
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

void ClearReplacedFunctions()
{
    ReplacedFunctions.clear();
}

GSC::GSC()
{
    Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
    Scr_GetFunction_Detour.Install();

    Scr_GetMethod_Detour = Detour(Scr_GetMethod, Scr_GetMethod_Hook);
    Scr_GetMethod_Detour.Install();

    // Intercept TU4 VM_Execute's opcode fetch to redirect registered function code positions, then resume at 0x82212E40.
    VM_Execute_Detour = Detour(reinterpret_cast<void*>(0x82212E30), reinterpret_cast<const void*>(VM_Execute_Hook));
    VM_Execute_Detour.Install();

    InitializeHudElemMethods();
}

void GSC::OnVMShutdown()
{
    ClearHudElemLocalizedStringState();
}

GSC::~GSC()
{
    ShutdownHudElemMethods();

    Scr_GetFunction_Detour.Remove();

    Scr_GetMethod_Detour.Remove();

    VM_Execute_Detour.Remove();
}
} // namespace mp
} // namespace iw3
