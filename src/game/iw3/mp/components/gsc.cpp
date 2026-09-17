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
    {"replaceFunc", GScr_ReplaceFunc, BUILTIN_ANY},
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

//replaceFunc
Detour VM_Execute_Detour;

const char* GetCodePosForParam(int index)
{
    const int numParams = static_cast<int>(Scr_GetNumParam());

    if (index < 0 || index >= numParams)
        return nullptr;

    auto* top = *reinterpret_cast<VariableValue**>(0x82BA6DF8);
    auto* value = top - index;

    if (value->type != VAR_FUNCTION)
        return nullptr;

    return value->codePosValue;
}

const char* IW3_GSC_ResolveReplaceFunc(const char* pos)
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

extern "C" const char* IW3_GSC_GetReplacedPos(const char* pos)
{
    const char* replacement = IW3_GSC_ResolveReplaceFunc(pos);

    if (replacement)
    {
        printf("[ReplaceFunc VM HIT] %p -> %p\n", pos, replacement);
        return replacement;
    }

    return pos;
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
        printf("replaceFunc: needs two parameters\n");
        return;
    }

    const char* what = GetCodePosForParam(0);
    const char* with = GetCodePosForParam(1);

    if (!what)
    {
        printf("replaceFunc: first parameter is not a function\n");
        return;
    }

    if (!with)
    {
        printf("replaceFunc: second parameter is not a function\n");
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

void ClearReplacedFunctions()
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

GSC::GSC()
{
    Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
    Scr_GetFunction_Detour.Install();

    Scr_GetMethod_Detour = Detour(Scr_GetMethod, Scr_GetMethod_Hook);
    Scr_GetMethod_Detour.Install();

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
