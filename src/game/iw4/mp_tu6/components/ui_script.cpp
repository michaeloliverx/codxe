#include "pch.h"
#include "ui_script.h"

namespace iw4
{
namespace mp_tu6
{
std::map<std::string, UIScript::Callback> UIScript::Scripts;
Detour UIScript::UI_RunMenuScript_Detour;

void UIScript::Add(const char *name, Callback callback)
{
    if (name && callback)
        Scripts[name] = callback;
}

void UIScript::Run(int localClientNum, const char **args, const char *actualScript)
{
    // Parse a copy: unhandled native scripts must receive their original args.
    const char *customArgs = args ? *args : nullptr;
    char name[1024];
    if (customArgs && String_Parse(&customArgs, name, sizeof(name)))
    {
        for (auto script = Scripts.begin(); script != Scripts.end(); ++script)
        {
            if (I_stricmp(name, script->first.c_str()) == 0)
            {
                script->second(localClientNum, &customArgs);
                if (args)
                    *args = customArgs;
                return;
            }
        }
    }
    UI_RunMenuScript_Detour.GetOriginal<UI_RunMenuScript_t>()(localClientNum, args, actualScript);
}

UIScript::UIScript()
{
    UI_RunMenuScript_Detour = Detour(UI_RunMenuScript, Run);
    UI_RunMenuScript_Detour.Install();
}

UIScript::~UIScript()
{
    UI_RunMenuScript_Detour.Remove();
    Scripts.clear();
}
} // namespace mp_tu6
} // namespace iw4
