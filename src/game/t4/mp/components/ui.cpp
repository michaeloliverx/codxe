#include "ui.h"
#include "common.h"

namespace t4
{
namespace mp
{

Detour Menus_OpenByName_Detour;

void Menus_OpenByName_Hook(UiContext *dc, const char *menuName)
{
    // Splitscreen map list only contains a subset of maps
    // The menu UI determines the map list based on the splitscreen setting
    if (strcmp(menuName, "menu_gamesetup_splitscreen") == 0)
        Cbuf_ExecuteBuffer(0, 0, "set splitscreen 0");

    Menus_OpenByName_Detour.GetOriginal<decltype(Menus_OpenByName)>()(dc, menuName);
}

Detour UI_RunMenuScript_Detour;

void UI_RunMenuScript_Hook(int localClientNum, const char **args, const char *actualScript)
{
    const char *argCopy = *args; // Clone the original pointer
    char parsedCommand[1024];

    if (!String_Parse(&argCopy, parsedCommand, sizeof(parsedCommand)))
        return;

    if (strcmp(parsedCommand, "StartServer") == 0)
    {
        // Splitscreen game will not load if splitscreen is not set to 1
        if (Menu_IsMenuOpenAndVisible(localClientNum, "menu_gamesetup_splitscreen"))
            Cbuf_ExecuteBuffer(0, 0, "set splitscreen 1");
    }

    UI_RunMenuScript_Detour.GetOriginal<decltype(UI_RunMenuScript)>()(localClientNum, args, actualScript);
}

ui::ui()
{
    Menus_OpenByName_Detour = Detour(Menus_OpenByName, Menus_OpenByName_Hook);
    Menus_OpenByName_Detour.Install();

    UI_RunMenuScript_Detour = Detour(UI_RunMenuScript, UI_RunMenuScript_Hook);
    UI_RunMenuScript_Detour.Install();
}

ui::~ui()
{
    Menus_OpenByName_Detour.Remove();

    UI_RunMenuScript_Detour.Remove();
}
} // namespace mp
} // namespace t4
