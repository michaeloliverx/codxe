#include "pch.h"
#include "g_scr_main.h"

namespace iw2
{
namespace mp
{
std::vector<BuiltinFunctionDef *> scr_functions;
std::vector<BuiltinMethodDef *> scr_methods;

// Inlined in the executable, reimplemented here
static unsigned int Scr_GetNumParam()
{
    return *scrVmPub_outparamcount;
}

void GScr_CbufAddText()
{
    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: exec(<string>)\n");

    const char *text = Scr_GetString(0);
    Cbuf_AddText(text);
}

int CL_IsKeyPressed(const char *keyName)
{
    const int keynum = Key_StringToKeynum(keyName);
    if (keynum >= 0)
        return (*keys)[keynum].down;
    else
        return 0;
}

void PlayerCmd_ButtonPressed(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    const char *button = Scr_GetString(0);

    if (!button || !*button)
        Scr_Error("usage: <client> buttonPressed(<button name>)");

    const int keypressed = CL_IsKeyPressed(button);
    return Scr_AddBool(keypressed);
}

void PlayerCmd_ADSButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddBool(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & CMD_BUTTON_ADS) != 0);
}

void PlayerCmd_JumpButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddBool(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & CMD_BUTTON_JUMP) != 0);
}

void PlayerCmd_FragButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddBool(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & CMD_BUTTON_FRAG) != 0);
}

void PlayerCmd_SmokeButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddBool(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & CMD_BUTTON_SMOKE) != 0);
}

static auto Scr_GetFunction = reinterpret_cast<BuiltinFunction (*)(const char **pName, int *type)>(0x823CB720);
static auto Scr_GetMethod = reinterpret_cast<BuiltinMethod (*)(const char **pName, int *type)>(0x823CB968);

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

BuiltinFunction Scr_GetFunction_Hook(const char **pName, int *type)
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

    return Scr_GetFunction_Detour.GetOriginal<decltype(Scr_GetFunction)>()(pName, type);
}

Detour Scr_GetMethod_Detour;

BuiltinMethod Scr_GetMethod_Hook(const char **pName, int *type)
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

    return Scr_GetMethod_Detour.GetOriginal<decltype(Scr_GetMethod)>()(pName, type);
}

g_scr_main::g_scr_main()
{
    Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
    Scr_GetFunction_Detour.Install();

    Scr_GetMethod_Detour = Detour(Scr_GetMethod, Scr_GetMethod_Hook);
    Scr_GetMethod_Detour.Install();

    Scr_AddFunction("exec", GScr_CbufAddText, BUILTIN_ANY);

    Scr_AddMethod("buttonpressed", PlayerCmd_ButtonPressed, BUILTIN_ANY); // Only works for host buttons

    Scr_AddMethod("adsbuttonpressed", PlayerCmd_ADSButtonPressed, BUILTIN_ANY);
    Scr_AddMethod("jumpbuttonpressed", PlayerCmd_JumpButtonPressed, BUILTIN_ANY);
    Scr_AddMethod("fragbuttonpressed", PlayerCmd_FragButtonPressed, BUILTIN_ANY);
    Scr_AddMethod("smokebuttonpressed", PlayerCmd_SmokeButtonPressed, BUILTIN_ANY);
}

g_scr_main::~g_scr_main()
{
    Scr_GetFunction_Detour.Remove();

    Scr_GetMethod_Detour.Remove();
}
} // namespace mp
} // namespace iw2
