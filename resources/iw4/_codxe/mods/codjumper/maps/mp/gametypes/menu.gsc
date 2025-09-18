#include common_scripts\utility;
#include maps\mp\gametypes\_hud_util;

init()
{
    // Virtual resolution for HUD elements; scaled to real monitor dimensions by the game engine
    level.SCREEN_MAX_WIDTH = 640;
    level.SCREEN_MAX_HEIGHT = 480;

    level.MENU_SCROLL_TIME_SECONDS = 0.250;

    level.THEMES = get_themes();
    level.MAPS = get_maps();
}

/**
 * Normalize RGB values (0-255) to (0-1).
 */
rgbToNormalized(rgb)
{
    return (rgb[0] / 255, rgb[1] / 255, rgb[2] / 255);
}

get_themes()
{
    themes = [];

    themes["blue"] = rgbToNormalized((0, 0, 255));
    themes["brown"] = rgbToNormalized((139, 69, 19));
    themes["cyan"] = rgbToNormalized((0, 255, 255));
    themes["gold"] = rgbToNormalized((255, 215, 0));
    themes["green"] = rgbToNormalized((0, 208, 98));
    themes["lime"] = rgbToNormalized((0, 255, 0));
    themes["magenta"] = rgbToNormalized((255, 0, 255));
    themes["maroon"] = rgbToNormalized((128, 0, 0));
    themes["olive"] = rgbToNormalized((128, 128, 0));
    themes["orange"] = rgbToNormalized((255, 165, 0));
    themes["pink"] = rgbToNormalized((255, 25, 127));
    themes["purple"] = rgbToNormalized((90, 0, 208));
    themes["red"] = rgbToNormalized((255, 0, 0));
    themes["salmon"] = rgbToNormalized((250, 128, 114));
    themes["silver"] = rgbToNormalized((192, 192, 192));
    themes["skyblue"] = rgbToNormalized((0, 191, 255));
    themes["tan"] = rgbToNormalized((210, 180, 140));
    themes["teal"] = rgbToNormalized((0, 128, 128));
    themes["turquoise"] = rgbToNormalized((64, 224, 208));
    themes["violet"] = rgbToNormalized((238, 130, 238));
    themes["yellow"] = rgbToNormalized((255, 255, 0));

    return themes;
}

get_maps()
{
    maps = [];
    // Alphabetically sorted by value
    maps["mp_afghan"] = "Afghan";
    maps["mp_complex"] = "Bailout";
    maps["mp_carnival"] = "Carnival";
    maps["mp_crash"] = "Crash";
    maps["mp_derail"] = "Derail";
    maps["mp_estate"] = "Estate";
    maps["mp_favela"] = "Favela";
    maps["mp_fuel2"] = "Fuel";
    maps["mp_highrise"] = "Highrise";
    maps["mp_invasion"] = "Invasion";
    maps["mp_checkpoint"] = "Karachi";
    maps["mp_overgrown"] = "Overgrown";
    maps["mp_quarry"] = "Quarry";
    maps["mp_abandon"] = "Rundown";
    maps["mp_rust"] = "Rust";
    maps["mp_compact"] = "Salvage";
    maps["mp_boneyard"] = "Scrapyard";
    maps["mp_nightshift"] = "Skidrow";
    maps["mp_storm"] = "Storm";
    maps["mp_strike"] = "Strike";
    maps["mp_subbase"] = "Sub Base";
    maps["mp_terminal"] = "Terminal";
    maps["mp_trailerpark"] = "Trailer Park";
    maps["mp_underpass"] = "Underpass";
    maps["mp_vacant"] = "Vacant";
    maps["mp_brecourt"] = "Wasteland";

    return maps;
}

/**
 * Add a menu to the menuOptions array.
 * @param menuKey The key to identify the menu.
 * @param parentMenuKey The key of the parent menu.
 */
addMenu(menuKey, parentMenuKey)
{
    if (!isdefined(self.menuOptions))
        self.menuOptions = [];
    self.menuOptions[menuKey] = spawnstruct();
    self.menuOptions[menuKey].parent = parentMenuKey;
    self.menuOptions[menuKey].options = [];
}

/**
 * Add a menu option to the menuOptions array.
 * @param menuKey The menu key to add the option to.
 * @param label The text to display for the option.
 * @param func The function to call when the option is selected.
 * @param param1 The first parameter to pass to the function. (optional)
 * @param param2 The second parameter to pass to the function. (optional)
 * @param param3 The third parameter to pass to the function. (optional)
 */
addMenuOption(menuKey, label, func, param1, param2, param3)
{
    option = spawnstruct();
    option.label = label;
    option.func = func;
    option.inputs = [];

    if (isdefined(param1))
        option.inputs[0] = param1;
    if (isdefined(param2))
        option.inputs[1] = param2;
    if (isdefined(param3))
        option.inputs[2] = param3;

    self.menuOptions[menuKey].options[self.menuOptions[menuKey].options.size] = option;
}

menuKeyExists(menuKey)
{
    return isdefined(self.menuOptions[menuKey]);
}

/**
 * Get the menu text for the current menu.
 */
getMenuText()
{
    if (!menuKeyExists(self.menuKey))
    {
        self iprintln("^1menu key " + self.menuKey + " does not exist");
        return "";
    }

    string = "";
    for (i = 0; i < self.menuOptions[self.menuKey].options.size; i++)
        string += self.menuOptions[self.menuKey].options[i].label + "\n";

    // hud elements can have a maximum of 255 characters otherwise they disappear
    if (string.size > 255)
        self iprintln("^1menu text exceeds 255 characters. current size: " + string.size);

    return string;
}

/**
 * Initialize the menu HUD elements.
 */
initMenuHudElem()
{
    menuWidth = int(level.SCREEN_MAX_WIDTH * 0.25); // force int because shaders dimensions won't work with floats
    menuTextPaddingLeft = 5;
    menuScrollerAlpha = 0.7;

    menuBackground = newClientHudElem(self);
    menuBackground.elemType = "icon";
    menuBackground.color = (0, 0, 0);
    menuBackground.alpha = 0.5;
    menuBackground setShader("white", menuWidth, level.SCREEN_MAX_HEIGHT);
    menuBackground.x = level.SCREEN_MAX_WIDTH - menuWidth;
    menuBackground.y = 0;
    menuBackground.alignX = "left";
    menuBackground.alignY = "top";
    menuBackground.horzAlign = "fullscreen";
    menuBackground.vertAlign = "fullscreen";
    self.menuBackground = menuBackground;

    leftBorderWidth = 2;

    menuBorderLeft = newClientHudElem(self);
    menuBorderLeft.elemType = "icon";
    menuBorderLeft.color = self.themeColor;
    menuBorderLeft.alpha = level.menuScrollerAlpha;
    menuBorderLeft setShader("white", leftBorderWidth, level.SCREEN_MAX_HEIGHT);
    menuBorderLeft.x = (level.SCREEN_MAX_WIDTH - menuWidth);
    menuBorderLeft.y = 0;
    menuBorderLeft.alignX = "left";
    menuBorderLeft.alignY = "top";
    menuBorderLeft.horzAlign = "fullscreen";
    menuBorderLeft.vertAlign = "fullscreen";
    self.menuBorderLeft = menuBorderLeft;

    menuScroller = newClientHudElem(self);
    menuScroller.elemType = "icon";
    menuScroller.color = self.themeColor;
    menuScroller.alpha = level.menuScrollerAlpha;
    menuScroller setShader("white", menuWidth, int(level.fontHeight * 1.2));
    menuScroller.x = level.SCREEN_MAX_WIDTH - menuWidth;
    menuScroller.y = int(level.SCREEN_MAX_HEIGHT * 0.15);
    menuScroller.alignX = "left";
    menuScroller.alignY = "top";
    menuScroller.horzAlign = "fullscreen";
    menuScroller.vertAlign = "fullscreen";
    self.menuScroller = menuScroller;

    menuTextFontElem = newClientHudElem(self);
    menuTextFontElem.elemType = "font";
    menuTextFontElem.font = "small";
    menuTextFontElem.fontscale = 1.2;
    menuTextFontElem settext(getMenuText());
    menuTextFontElem.x = (level.SCREEN_MAX_WIDTH - menuWidth) + menuTextPaddingLeft;
    menuTextFontElem.y = int(level.SCREEN_MAX_HEIGHT * 0.15);
    menuTextFontElem.alignX = "left";
    menuTextFontElem.alignY = "top";
    menuTextFontElem.horzAlign = "fullscreen";
    menuTextFontElem.vertAlign = "fullscreen";
    self.menuTextFontElem = menuTextFontElem;

    menuHeaderFontElem = newClientHudElem(self);
    menuHeaderFontElem.elemType = "font";
    menuHeaderFontElem.font = "objective";
    menuHeaderFontElem.fontscale = 2;
    menuHeaderFontElem.glowColor = self.themeColor;
    menuHeaderFontElem.glowAlpha = 1;
    menuHeaderFontElem.x = (level.SCREEN_MAX_WIDTH - menuWidth) + menuTextPaddingLeft;
    menuHeaderFontElem.y = int(level.SCREEN_MAX_HEIGHT * 0.025);
    menuHeaderFontElem.alignX = "left";
    menuHeaderFontElem.alignY = "top";
    menuHeaderFontElem.horzAlign = "fullscreen";
    menuHeaderFontElem.vertAlign = "fullscreen";
    menuHeaderFontElem settext("CodJumper");
    self.menuHeaderFontElem = menuHeaderFontElem;

    menuHeaderAuthorFontElem = newClientHudElem(self);
    menuHeaderAuthorFontElem.elemType = "font";
    menuHeaderAuthorFontElem.font = "default";
    menuHeaderAuthorFontElem.fontscale = 1.2;
    menuHeaderAuthorFontElem.glowColor = self.themeColor;
    menuHeaderAuthorFontElem.glowAlpha = 0.1;
    menuHeaderAuthorFontElem.x = (level.SCREEN_MAX_WIDTH - menuWidth) + menuTextPaddingLeft;
    menuHeaderAuthorFontElem.y = int(level.SCREEN_MAX_HEIGHT * 0.075);
    menuHeaderAuthorFontElem.alignX = "left";
    menuHeaderAuthorFontElem.alignY = "top";
    menuHeaderAuthorFontElem.horzAlign = "fullscreen";
    menuHeaderAuthorFontElem.vertAlign = "fullscreen";
    menuHeaderAuthorFontElem settext("by mo");
    self.menuHeaderAuthorFontElem = menuHeaderAuthorFontElem;
}

/**
 * Handle menu actions.
 * @param action The action to perform.
 * @param param1 The action parameter. (optional)
 */
menuAction(action, param1)
{
    if (!isdefined(self.cj.menu_open))
        self.cj.menu_open = false;

    if (!isdefined(self.themeColor))
        self.themeColor = level.THEMES["skyblue"];

    if (!isdefined(self.menuKey))
        self.menuKey = "main";

    if (!isdefined(self.menuCursor))
        self.menuCursor = [];

    if (!isdefined(self.menuCursor[self.menuKey]))
        self.menuCursor[self.menuKey] = 0;

    switch (action)
    {
    case "UP":
    case "DOWN":
        if (action == "UP")
            self.menuCursor[self.menuKey]--;
        else if (action == "DOWN")
            self.menuCursor[self.menuKey]++;

        if (self.menuCursor[self.menuKey] < 0)
            self.menuCursor[self.menuKey] = self.menuOptions[self.menuKey].options.size - 1;
        else if (self.menuCursor[self.menuKey] > self.menuOptions[self.menuKey].options.size - 1)
            self.menuCursor[self.menuKey] = 0;

        self.menuScroller moveOverTime(level.MENU_SCROLL_TIME_SECONDS);
        self.menuScroller.y =
            (level.SCREEN_MAX_HEIGHT * 0.15 + ((level.fontHeight * 1.2) * self.menuCursor[self.menuKey]));
        break;
    case "SELECT":
        cursor = self.menuCursor[self.menuKey];
        options = self.menuOptions[self.menuKey].options[cursor];
        if (options.inputs.size == 0)
            self [[options.func]] ();
        else if (options.inputs.size == 1)
            self [[options.func]] (options.inputs[0]);
        else if (options.inputs.size == 2)
            self [[options.func]] (options.inputs[0], options.inputs[1]);
        else if (options.inputs.size == 3)
            self [[options.func]] (options.inputs[0], options.inputs[1], options.inputs[2]);
        wait 0.1;
        break;
    case "CLOSE":
        // TODO: check can .children be used to destroy all at once
        self.menuBackground destroy();
        self.menuBorderLeft destroy();
        self.menuScroller destroy();
        self.menuTextFontElem destroy();
        self.menuHeaderFontElem destroy();
        self.menuHeaderAuthorFontElem destroy();
        self.cj.menu_open = false;
        self freezecontrols(false);
        break;
    case "BACK":
        // close menu if we don't have a parent
        if (!isdefined(self.menuOptions[self.menuKey].parent))
            self menuAction("CLOSE");
        else
            self menuAction("CHANGE_MENU", self.menuOptions[self.menuKey].parent);
        break;
    case "OPEN":
        self.cj.menu_open = true;
        self freezecontrols(true);
        self generateMenuOptions();
        self initMenuHudElem();
        self.menuScroller.y =
            (level.SCREEN_MAX_HEIGHT * 0.15 + ((level.fontHeight * 1.2) * self.menuCursor[self.menuKey]));
        break;
    case "CHANGE_THEME":
        self.themeColor = level.THEMES[param1];
        self menuAction("REFRESH");
        break;
    case "CHANGE_MENU":
        self.menuKey = param1;
        self menuAction("REFRESH_TEXT");
        break;
    case "REFRESH_TEXT":
        // sanity check to prevent crashing
        if (!menuKeyExists(self.menuKey))
        {
            self iprintln("^1menu key " + self.menuKey + " does not exist");
            self.menuKey = "main";
        }
        self.menuTextFontElem settext(getMenuText());
        self.menuScroller moveOverTime(level.MENU_SCROLL_TIME_SECONDS);
        self.menuScroller.y =
            (level.SCREEN_MAX_HEIGHT * 0.15 + ((level.fontHeight * 1.2) * self.menuCursor[self.menuKey]));
        break;
    case "REFRESH":
        self menuAction("CLOSE");
        self menuAction("OPEN");
        break;
    default:
        self iprintln("^1unknown menu action " + action);
        break;
    }
}

isMenuOpen()
{
    return self.cj.menu_open == true;
}

generateMenuOptions()
{
    self addMenu("main");

    // Map selector
    self addMenuOption("main", "Maps", ::menuAction, "CHANGE_MENU", "host_menu_maps");
    self addMenu("host_menu_maps", "main");
    maps = getarraykeys(level.MAPS);
    for (i = maps.size - 1; i >= 0; i--) // loop in reverse to display the maps in the order they are defined
    {
        mapname = maps[i];
        label = level.MAPS[mapname];
        self addMenuOption("host_menu_maps", label, ::changeMap, mapname);
    }

    self addMenuOption("main", "Themes", ::menuAction, "CHANGE_MENU", "theme_menu");
    self addMenu("theme_menu", "main");
    themes = getarraykeys(level.THEMES);
    for (i = themes.size - 1; i >= 0; i--) // reverse order to display in the order they are defined
        self addMenuOption("theme_menu", themes[i], ::menuAction, "CHANGE_THEME", themes[i]);

    self addMenuOption("main", "Toggle Old School Mode", ::ToggleOldschool);

    self addMenuOption("main", "Clone Menu", ::menuAction, "CHANGE_MENU", "clone_menu");
    self addMenu("clone_menu", "main");
    self addMenuOption("clone_menu", "Spawn Clone", ::AddClone);
    self addMenuOption("clone_menu", "Remove Clones", ::RemoveAllClones);

    self addMenuOption("main", "Barrier Menu", ::menuAction, "CHANGE_MENU", "barrier_menu");
    self addMenu("barrier_menu", "main");
    self addMenuOption("barrier_menu", "Restore Barriers", ::RestoreBrushCollision);
    self addMenuOption("barrier_menu", "Disable Collision at player position", ::DisablePlayerClipOnIntersectingBrushes);
}

changeMap(mapname)
{
    Map(mapname);
}

ToggleOldschool()
{
    jump_height = getDvarInt("jump_height");
    if (jump_height == 39)
    {
        setDvar("jump_height", 64);
        iPrintln("Old School Mode [^2ON^7]");
    }
    else
    {
        setDvar("jump_height", 39);
        iPrintln("Old School Mode [^1OFF^7]");
    }
}

AddClone()
{
    if (!isdefined(self.cj.clones))
        self.cj.clones = [];

    body = self ClonePlayer(100000);
    self.cj.clones[self.cj.clones.size] = body;
}

RemoveAllClones()
{
    if (!isdefined(self.cj.clones))
        self.cj.clones = [];

    foreach (clone in self.cj.clones)
        clone delete ();
}

RestoreBrushCollision()
{
	SetDvar("noclip_brushes", "");
	IPrintLn("^2Collision restored for all brushes.");
}
