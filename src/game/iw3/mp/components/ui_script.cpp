#include "pch.h"
#include "ui_script.h"

#include "mods.h"
#include "stats.h"
#include "ui_feeder.h"

namespace iw3
{
namespace mp
{
const UIScriptEntry UIScript::Scripts[] = {
    {"LoadMods", ModList::LoadModsScript},          {"RunMod", ModList::RunModScript},
    {"ClearMods", ModList::ClearModsScript},        {"LoadOfflineStats", Stats::LoadStatsScript},
    {"LoadUsermaps", UIFeeder::LoadUsermapsScript}, {"ApplyMap", UIFeeder::ApplyMapScript},
};

Detour UIScript::UI_RunMenuScript_Detour;

UIScript::UIScript()
{
    UI_RunMenuScript_Detour = Detour(UI_RunMenuScript, UI_RunMenuScript_Hook);
    UI_RunMenuScript_Detour.Install();
}

UIScript::~UIScript()
{
    UI_RunMenuScript_Detour.Remove();
}

bool UIScript::Run(int localClientNum, const char **args)
{
    if (!args || !*args)
        return false;

    const char *customArgs = *args;
    char name[1024];
    if (!String_Parse(&customArgs, name, static_cast<int>(sizeof(name))))
        return false;

    for (size_t i = 0; i < ARRAYSIZE(Scripts); ++i)
    {
        if (I_stricmp(name, Scripts[i].name) == 0)
        {
            Scripts[i].callback(localClientNum, &customArgs);
            return true;
        }
    }

    return false;
}

void UIScript::UI_RunMenuScript_Hook(int localClientNum, const char **args, const char *actualScript)
{
    if (Run(localClientNum, args))
        return;

    UI_RunMenuScript_Detour.GetOriginal<UI_RunMenuScript_t>()(localClientNum, args, actualScript);
}
} // namespace mp
} // namespace iw3
