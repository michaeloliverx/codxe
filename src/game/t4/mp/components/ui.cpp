#include "pch.h"
#include "ui.h"
#include "stats.h"
#include "mods.h"

namespace t4
{
namespace mp
{

Detour Menus_AddMenuList_Detour;

void Menus_AddMenuList_Hook(UiContext *dc, MenuList *menuList)
{
    auto original = Menus_AddMenuList_Detour.GetOriginal<decltype(Menus_AddMenuList)>();
    original(dc, menuList);

    if (!menuList || !menuList->name || strcmp(menuList->name, "ui_mp/menus.txt") != 0)
        return;

    // Append before UI_Init initializes the registered menus and their item parents.
    // Rebuilding the stock list with reference assets can retain default menu stubs.
    auto mods =
        reinterpret_cast<MenuList *>(DB_FindXAssetHeader(ASSET_TYPE_MENULIST, "ui_mp/mods_menu.menu", false, 0));
    if (mods && mods->name && strcmp(mods->name, "ui_mp/mods_menu.menu") == 0)
    {
        original(dc, mods);
        DbgPrint("[codxe][T4][UI] Registered mod browser alongside stock frontend menus\n");
    }
    else
    {
        DbgPrint("[codxe][T4][UI] Mod browser menulist is unavailable\n");
    }
}

Detour Menus_OpenByName_Detour;

void Menus_OpenByName_Hook(UiContext *dc, const char *menuName)
{
    if (strcmp(menuName, "main") == 0 || strncmp(menuName, "menu_xboxlive", 13) == 0)
        stats::RestoreLiveStats();

    // Splitscreen map list only contains a subset of maps
    // The menu UI determines the map list based on the splitscreen setting
    if (strcmp(menuName, "menu_gamesetup_splitscreen") == 0)
        Cbuf_ExecuteBuffer(0, 0, "set splitscreen 0");

    Menus_OpenByName_Detour.GetOriginal<decltype(Menus_OpenByName)>()(dc, menuName);

    // Increase the maximum number of clients in xboxlive private match and systemlink from 18 > 24
    if (strcmp(menuName, "menu_xboxlive_privatelobby") == 0)
    {
        Cbuf_AddText(0, "set party_maxplayers 24");
    }
    else if (strcmp(menuName, "menu_gamesetup_systemlink") == 0)
    {
        Cbuf_AddText(0, "set sv_maxclients 24");
    }
}

Detour UI_RunMenuScript_Detour;

void UI_RunMenuScript_Hook(int localClientNum, const char **args, const char *actualScript)
{
    const char *argCopy = *args; // Clone the original pointer
    char parsedCommand[1024];

    if (!String_Parse(&argCopy, parsedCommand, sizeof(parsedCommand)))
        return;

    if (ModList::RunMenuScript(localClientNum, parsedCommand))
    {
        // Item_RunScript resumes at this cursor after our custom command returns.
        *args = argCopy;
        return;
    }

    if (strcmp(parsedCommand, "LoadOfflineStats") == 0)
    {
        *args = argCopy;
        stats::LoadOfflineStats(localClientNum);
        return;
    }

    if (strcmp(parsedCommand, "StartServer") == 0)
    {
        if (!Dvar_GetBool("onlinegame"))
        {
            for (int client = 0; client < 4; ++client)
            {
                const int controller = CL_ControllerIndexFromClientNum(client);
                if (controller >= 0 && controller < 4 &&
                    (client == localClientNum || XUserGetSigninState(controller) != eXUserSigninState_NotSignedIn))
                    stats::LoadOfflineStats(client);
            }
        }

        // Splitscreen game will not load if splitscreen is not set to 1
        if (Menu_IsMenuOpenAndVisible(localClientNum, "menu_gamesetup_splitscreen"))
            Cbuf_ExecuteBuffer(0, 0, "set splitscreen 1");
    }

    UI_RunMenuScript_Detour.GetOriginal<decltype(UI_RunMenuScript)>()(localClientNum, args, actualScript);
}

ui::ui()
{
    Menus_AddMenuList_Detour = Detour(Menus_AddMenuList, Menus_AddMenuList_Hook);
    Menus_AddMenuList_Detour.Install();

    Menus_OpenByName_Detour = Detour(Menus_OpenByName, Menus_OpenByName_Hook);
    Menus_OpenByName_Detour.Install();

    UI_RunMenuScript_Detour = Detour(UI_RunMenuScript, UI_RunMenuScript_Hook);
    UI_RunMenuScript_Detour.Install();
}

ui::~ui()
{
    Menus_AddMenuList_Detour.Remove();

    Menus_OpenByName_Detour.Remove();

    UI_RunMenuScript_Detour.Remove();
}
} // namespace mp
} // namespace t4
