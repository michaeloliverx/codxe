#include "pch.h"
#include "gsc.h"
#include "common/gsc_registry.h"
#include <unordered_map>

namespace t4
{
namespace sp
{
static std::unordered_map<const char*, const char*> ReplacedFunctionsSP;
namespace
{
client_t *VM_GetClientForEntRef(scr_entref_t entref)
{
    return &(*reinterpret_cast<client_t **>(0x839EC08C))[entref.entnum];
}

void GScr_SpawnCollision()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 4)
        Scr_Error("Usage: SpawnCollision(<model>, <targetname>, <origin>, <angles>)", SCRIPTINSTANCE_SERVER);

    const char *modelName = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    const char *targetName = Scr_GetString(1, SCRIPTINSTANCE_SERVER);
    float origin[3];
    float angles[3];
    Scr_GetVector(2, origin, SCRIPTINSTANCE_SERVER, -1);
    Scr_GetVector(3, angles, SCRIPTINSTANCE_SERVER, -1);

    gentity_s *ent = G_Spawn();
    Scr_SetString(&ent->classname, GScr_AllocString("script_model"), SCRIPTINSTANCE_SERVER);
    ent->s.index = G_ModelIndex(modelName);
    if (!ent->s.index)
        Scr_ParamError(0, va("SpawnCollision: Collision model name %s is not valid.", modelName),
                       SCRIPTINSTANCE_SERVER);

    Scr_SetString(&ent->targetname, GScr_AllocString(targetName), SCRIPTINSTANCE_SERVER);
    std::memcpy(ent->r.currentOrigin, origin, sizeof(origin));
    std::memcpy(ent->r.currentAngles, angles, sizeof(angles));
    G_CallSpawnEntity(ent);
    ent->flags |= 0x200;
    Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_JumpButtonPressed(scr_entref_t entref)
{
    if (entref.classnum != 0)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

    const gentity_s *ent = &g_entities[entref.entnum];
    if (!ent->client)
        Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> JumpButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_GOSTAND) != 0, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_secondaryOffhandButtonPressed(scr_entref_t entref)
{
    if (entref.classnum != 0)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

    const gentity_s *ent = &g_entities[entref.entnum];
    if (!ent->client)
        Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> SecondaryOffhandButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_SMOKE) != 0, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_SprintButtonPressed(scr_entref_t entref)
{
    if (entref.classnum != 0)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

    const gentity_s *ent = &g_entities[entref.entnum];
    if (!ent->client)
        Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> SprintButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_SPRINT) != 0, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_MoveForwardButtonPressed(scr_entref_t entref)
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> MoveForwardButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    const client_t *cl = VM_GetClientForEntRef(entref);

    if (!cl)
        Scr_ObjectError("not a client\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(cl->lastUsercmd.forwardmove > 0, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_MoveBackButtonPressed(scr_entref_t entref)
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> MoveBackButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    const client_t *cl = VM_GetClientForEntRef(entref);

    if (!cl)
        Scr_ObjectError("not a client\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(cl->lastUsercmd.forwardmove < 0, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_MoveLeftButtonPressed(scr_entref_t entref)
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> MoveLeftButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    const client_t *cl = VM_GetClientForEntRef(entref);

    if (!cl)
        Scr_ObjectError("not a client\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(cl->lastUsercmd.rightmove < 0, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_MoveRightButtonPressed(scr_entref_t entref)
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> MoveRightButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    const client_t *cl = VM_GetClientForEntRef(entref);

    if (!cl)
        Scr_ObjectError("not a client\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(cl->lastUsercmd.rightmove > 0, SCRIPTINSTANCE_SERVER);
}

void ScriptEntCmd_CloneBrushModelToScriptModel(scr_entref_t entref)
{
    const int script_brushmodel = GScr_AllocString("script_brushmodel");
    const int script_model = GScr_AllocString("script_model");
    const int script_origin = GScr_AllocString("script_origin");
    const int light = GScr_AllocString("light");

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("usage: <scriptModelEnt> CloneBrushModelToScriptModel(<brushModelEnt>)", SCRIPTINSTANCE_SERVER);

    gentity_s *scriptEnt = &g_entities[entref.entnum];
    if (scriptEnt->classname != (unsigned short)script_model)
        Scr_ObjectError("passed entity is not a script_model entity", SCRIPTINSTANCE_SERVER);

    if (scriptEnt->s.eType != ET_SCRIPTMOVER)
        Scr_ObjectError("passed entity type is not 6 (TODO: what is it?)", SCRIPTINSTANCE_SERVER);

    const gentity_s *brushEnt = Scr_GetEntity(0);
    if (brushEnt->classname != (unsigned short)script_brushmodel &&
        brushEnt->classname != (unsigned short)script_model && brushEnt->classname != (unsigned short)script_origin &&
        brushEnt->classname != (unsigned short)light)
        Scr_ParamError(
            0, "brush model entity classname must be one of {script_brushmodel, script_model, script_origin, light}",
            SCRIPTINSTANCE_SERVER);

    if (!brushEnt->s.index)
        Scr_ParamError(0, "brush model entity has no collision model", SCRIPTINSTANCE_SERVER);

    SV_UnlinkEntity(scriptEnt);
    scriptEnt->s.index = brushEnt->s.index;
    int contents = scriptEnt->r.contents;
    SV_SetBrushModel(scriptEnt);
    scriptEnt->r.contents |= contents;
    SV_LinkEntity(scriptEnt);
}

static const char* GetCodePosForParam(int index)
{
    const scriptInstance_t inst = SCRIPTINSTANCE_SERVER;
    const int numParams = static_cast<int>(Scr_GetNumParam(inst));

    if (index < 0 || index >= numParams)
        return nullptr;

    auto* top = *reinterpret_cast<VariableValue**>(0x84B2F214 + (static_cast<int>(inst) * 0x14));

    if (!top)
        return nullptr;

    auto* value = top - index;

    if (value->type != VAR_FUNCTION)
        return nullptr;

    return value->u.codePosValue;
}
static void GScr_ReplaceFunc()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 2)
    {
        Scr_Error("replaceFunc: needs two parameters", SCRIPTINSTANCE_SERVER);
        return;
    }

    const char* what = GetCodePosForParam(0);
    const char* with = GetCodePosForParam(1);

    if (!what)
    {
        Scr_Error("replaceFunc: first parameter is not a function", SCRIPTINSTANCE_SERVER);
        return;
    }

    if (!with)
    {
        Scr_Error("replaceFunc: second parameter is not a function", SCRIPTINSTANCE_SERVER);
        return;
    }

    ReplacedFunctionsSP[what] = with;
}

extern "C" const char* T4SP_GSC_GetReplacedPos(const char* pos)
{
    if (!pos)
        return pos;

    const auto it = ReplacedFunctionsSP.find(pos);

    if (it != ReplacedFunctionsSP.end())
        return it->second;

    return pos;
}

Detour VMExecuteInternal_SP_Detour;

extern "C" __declspec(naked) void VM_Execute_Hook_SP()
{
    __asm
    {
        mr      r3, r10
        bl      T4SP_GSC_GetReplacedPos
        mr      r10, r3

        addi    r11, r10, 1
        lbz     r9, 0(r10)
        addi    r10, r27, -0x4068
        slwi    r21, r31, 2

        lis     r12, 0x8234
        ori     r12, r12, 0x2EA8
        mtctr   r12
        bctr
    }
}

Detour Scr_ShutdownSystem_SP_Detour;

static int Scr_ShutdownSystem_Hook_SP(scriptInstance_t inst, int sys, int bComplete)
{
    if (inst == SCRIPTINSTANCE_SERVER && bComplete)
    {
        ReplacedFunctionsSP.clear();
    }

    return Scr_ShutdownSystem_SP_Detour.GetOriginal<Scr_ShutdownSystem_t>()(inst, sys, bComplete);
}

static const gsc::Entry<BuiltinMethod> methods[] = {
    {"jumpbuttonpressed", PlayerCmd_JumpButtonPressed, BUILTIN_ANY},
    {"secondaryoffhandbuttonpressed", PlayerCmd_secondaryOffhandButtonPressed, BUILTIN_ANY},
    {"sprintbuttonpressed", PlayerCmd_SprintButtonPressed, BUILTIN_ANY},
    {"moveforwardbuttonpressed", PlayerCmd_MoveForwardButtonPressed, BUILTIN_ANY},
    {"movebackbuttonpressed", PlayerCmd_MoveBackButtonPressed, BUILTIN_ANY},
    {"moveleftbuttonpressed", PlayerCmd_MoveLeftButtonPressed, BUILTIN_ANY},
    {"moverightbuttonpressed", PlayerCmd_MoveRightButtonPressed, BUILTIN_ANY},
    {"clonebrushmodeltoscriptmodel", ScriptEntCmd_CloneBrushModelToScriptModel, BUILTIN_ANY},
};

static const gsc::Entry<BuiltinFunction> functions[] = {
    {"spawncollision", GScr_SpawnCollision, BUILTIN_ANY},
    {"replaceFunc", GScr_ReplaceFunc, BUILTIN_ANY},
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

GSC::GSC()
{
    Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
    Scr_GetFunction_Detour.Install();

    Scr_GetMethod_Detour = Detour(Scr_GetMethod, Scr_GetMethod_Hook);
    Scr_GetMethod_Detour.Install();

    VMExecuteInternal_SP_Detour = Detour(reinterpret_cast<void*>(0x82342E98), reinterpret_cast<const void*>(VM_Execute_Hook_SP));
    VMExecuteInternal_SP_Detour.Install();

    Scr_ShutdownSystem_SP_Detour = Detour(reinterpret_cast<void*>(0x82341D20), reinterpret_cast<const void*>(Scr_ShutdownSystem_Hook_SP));
    Scr_ShutdownSystem_SP_Detour.Install();
}

GSC::~GSC()
{
    Scr_GetFunction_Detour.Remove();

    Scr_GetMethod_Detour.Remove();

    VMExecuteInternal_SP_Detour.Remove();

    Scr_ShutdownSystem_SP_Detour.Remove();
}
} // namespace sp
} // namespace t4
