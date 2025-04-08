#include common_scripts\utility;
#include maps\mp\gametypes\_hud_util;
#include maps\mp\gametypes\koth;
#include maps\mp\gametypes\sab;
#include maps\mp\gametypes\sd;

init()
{
	precacheShader("reticle_flechette"); // Precache the reticle shader for Forge

	// Replaced by the build script
	level.VERSION = "__VERSION__";

	// Virtual resolution for HUD elements; scaled to real monitor dimensions by the game engine
	level.SCREEN_MAX_WIDTH = 640;
	level.SCREEN_MAX_HEIGHT = 480;

	level.MENU_SCROLL_TIME_SECONDS = 0.250;

	level.DVARS = get_dvars();
	level.THEMES = get_themes();
	level.PLAYER_MODELS = get_player_models();
	level.MAPS = get_maps();

	level.SELECTED_PREFIX = "^2-->^7 ";

	level.FORGE_MODELS = get_forge_models();

	// sab_bomb is always on the ground in the middle of the map
	level.MAP_CENTER_GROUND_ORIGIN = getent("sab_bomb", "targetname").origin;

	setAllSpawnPointsToOrigin(level.MAP_CENTER_GROUND_ORIGIN);

	deleteUselessEntities();

	level.forge_change_modes[0] = "pitch";
	level.forge_change_modes[1] = "yaw";
	level.forge_change_modes[2] = "roll";
	level.forge_change_modes[3] = "z";

	level.hardcoreMode = true; // Force hardcore mode

	gametype = level.gametype;

	setDvar("scr_" + gametype + "_scorelimit", 0);
	setDvar("scr_" + gametype + "_timelimit", 0);
	setDvar("scr_" + gametype + "_playerrespawndelay", 0);
	setDvar("scr_" + gametype + "_numlives", 0);
	setDvar("scr_" + gametype + "_roundlimit", 0);

	setDvar("ui_hud_showobjicons", 0); // Hide objective icons from HUD and map

	setDvar("scr_game_perks", 0);		// Remove perks
	setDvar("scr_showperksonspawn", 0); // Remove perks icons shown on spawn
	setDvar("scr_game_hardpoints", 0);	// Remove killstreaks

	setDvar("player_sprintUnlimited", 1);
	setDvar("jump_slowdownEnable", 0);

	// Remove fall damage
	setDvar("bg_fallDamageMaxHeight", 9999);
	setDvar("bg_fallDamageMinHeight", 9998);

	// Prevent bots from moving
	setDvar("sv_botsPressAttackBtn", 0);

	setDvar("userinfo", "L"); // prevent people from freezing consoles via userinfo command

	// prevent dynents from moving from bullets / explosions
	setDvar("dynEnt_active", 0);
	setDvar("dynEnt_bulletForce", 0);
	setDvar("dynEnt_explodeForce", 0);
	setDvar("dynEnt_explodeMaxEnts", 0);
	setDvar("dynEnt_explodeMinForce", 9999999999);
	setDvar("dynEnt_explodeSpinScale", 0);
	setDvar("dynEnt_explodeUpbias", 0);
	setDvar("dynEntPieces_angularVelocity", 0);
	setDvar("dynEntPieces_impactForce", 0);
	setDvar("dynEntPieces_velocity", 0);

	level thread onPlayerConnect();
}

onPlayerConnect()
{
	for (;;)
	{
		level waittill("connecting", player);

		// Don't setup bot players
		if (isDefined(player.pers["isBot"]))
			continue;

		// JumpCrouch / binds helper
		player setClientDvar("activeaction", "vstr VSTR_LEAN_DISABLED;");
		player setClientDvar("VSTR_LEAN_ENABLED", "bind BUTTON_A vstr BUTTON_A_ACTION;bind DPAD_DOWN +actionslot 3; bind DPAD_LEFT +leanleft; bind DPAD_RIGHT +leanright");
		player setClientDvar("VSTR_LEAN_DISABLED", "bind BUTTON_A vstr BUTTON_A_ACTION;bind DPAD_DOWN +actionslot 2; bind DPAD_LEFT +actionslot 3; bind DPAD_RIGHT +actionslot 4");
		player setClientDvar("BUTTON_A_ACTION", "+gostand;-gostand");

		player setupPlayer();
		player thread onPlayerSpawned();
	}
}

onPlayerSpawned()
{
	self endon("disconnect");
	for (;;)
	{
		self waittill("spawned_player");

		self cj_setup_loadout();
		self thread replenish_ammo();
		self thread watch_buttons();
		self resetFOV();
	}
}

resetFOV()
{
	if (isdefined(self.cj["settings"]["cg_fov"]))
		self setClientDvar("cg_fov", self.cj["settings"]["cg_fov"]);
}

setupPlayer()
{
	self.cj = [];
	self.cj["bots"] = [];
	self.cj["botnumber"] = 0;
	self.cj["clones"] = [];
	self.cj["maxbots"] = 4;
	self.cj["savenum"] = 0;
	self.cj["saves"] = [];
	self.cj["settings"] = [];
	self.cj["settings"]["rpg_switch_enabled"] = false;
	self.cj["settings"]["rpg_switched"] = false;

	self.cj["meter_hud"] = [];

	self.cj["menu_open"] = false;

	self.cj["spectator_speed_index"] = 5;
	self.cj["forge_change_mode_index"] = 0;

	self.cj["dvars"] = [];

	// Default loadout
	self.cj["loadout"] = spawnstruct();
	self.cj["loadout"].primary = "mp5_mp";
	self.cj["loadout"].primaryCamoIndex = 0;
	self.cj["loadout"].sidearm = "deserteaglegold_mp";
	self.cj["loadout"].fastReload = false;
	self.cj["loadout"].incomingWeapon = undefined;

	// Remove unlocalized errors
	self setClientDvars("loc_warnings", 0, "loc_warningsAsErrors", 0, "cg_errordecay", 1, "con_errormessagetime", 0, "uiscript_debug", 0);

	// Set team names
	self setClientDvars("g_TeamName_Allies", "Jumpers", "g_TeamName_Axis", "Bots");

	self setClientDvars("cg_overheadRankSize", 0, "cg_overheadIconSize", 0); // Remove overhead rank and icon

	self setClientDvar("nightVisionDisableEffects", 1); // Remove nightvision fx

	// Remove objective waypoints on screen
	self setClientDvar("waypointIconWidth", 0.1);
	self setClientDvar("waypointIconHeight", 0.1);
	self setClientDvar("waypointOffscreenPointerWidth", 0.1);
	self setClientDvar("waypointOffscreenPointerHeight", 0.1);

	// Disable FX
	self setClientDvars("fx_enable", 0, "fx_marks", 0, "fx_marks_ents", 0, "fx_marks_smodels", 0);

	self setClientDvar("clanname", ""); // Remove clan tag
	self setClientDvar("motd", "CodJumper");

	self setClientDvar("aim_automelee_range", 0); // Remove melee lunge

	// Disable autoaim for enemy players
	self setClientDvars("aim_slowdown_enabled", 0, "aim_lockon_enabled", 0);

	// Don't show enemy player names
	self setClientDvars("cg_enemyNameFadeIn", 0, "cg_enemyNameFadeOut", 0);

	// Always show enemies on the map but hide compass, can see enemy positions when pressing start
	self setClientDvars("g_compassShowEnemies", 1, "compassSize", 0.001);

	self setClientDvar("cg_scoreboardPingText", 1);

	self setClientDvar("cg_chatHeight", 0); // prevent people from freezing consoles via say command

	// look straight up
	self setclientdvar("player_view_pitch_up", 89.9);

	// Remove glow color applied to the mode and map name strings on the connect screen
	self setClientDvar("ui_ConnectScreenTextGlowColor", 0);

	self setClientDvar("cg_descriptiveText", 0);		  // Remove spectator button icons and text
	self setClientDvar("player_spectateSpeedScale", 1.5); // Faster movement in spectator/ufo
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
	menuScroller setShader("white", menuWidth, int(level.fontHeight * 1.5));
	menuScroller.x = level.SCREEN_MAX_WIDTH - menuWidth;
	menuScroller.y = int(level.SCREEN_MAX_HEIGHT * 0.15);
	menuScroller.alignX = "left";
	menuScroller.alignY = "top";
	menuScroller.horzAlign = "fullscreen";
	menuScroller.vertAlign = "fullscreen";
	self.menuScroller = menuScroller;

	menuTextFontElem = newClientHudElem(self);
	menuTextFontElem.elemType = "font";
	menuTextFontElem.font = "default";
	menuTextFontElem.fontscale = 1.5;
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
	menuHeaderAuthorFontElem.fontscale = 1.5;
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

	menuVersionFontElem = newClientHudElem(self);
	menuVersionFontElem.elemType = "font";
	menuVersionFontElem.font = "default";
	menuVersionFontElem.fontscale = 1.4;
	menuVersionFontElem.alpha = 0.5;
	menuVersionFontElem.x = (level.SCREEN_MAX_WIDTH - menuWidth) + menuTextPaddingLeft;
	menuVersionFontElem.y = int(level.SCREEN_MAX_HEIGHT - (level.fontHeight * menuVersionFontElem.fontscale) - menuTextPaddingLeft);
	menuVersionFontElem.alignX = "left";
	menuVersionFontElem.alignY = "top";
	menuVersionFontElem.horzAlign = "fullscreen";
	menuVersionFontElem.vertAlign = "fullscreen";
	menuVersionFontElem settext(level.VERSION);
	self.menuVersionFontElem = menuVersionFontElem;
}

/**
 * Handle menu actions.
 * @param action The action to perform.
 * @param param1 The action parameter. (optional)
 */
menuAction(action, param1)
{
	// if (!isdefined(self.cj["menu_open"]))
	// 	self.cj["menu_open"] = false;

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
		self.menuScroller.y = (level.SCREEN_MAX_HEIGHT * 0.15 + ((level.fontHeight * 1.5) * self.menuCursor[self.menuKey]));
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
		self.menuVersionFontElem destroy();
		self.cj["menu_open"] = false;
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
		self.cj["menu_open"] = true;
		self freezecontrols(true);
		self generateMenuOptions();
		self initMenuHudElem();
		self.menuScroller.y = (level.SCREEN_MAX_HEIGHT * 0.15 + ((level.fontHeight * 1.5) * self.menuCursor[self.menuKey]));
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
		self.menuScroller.y = (level.SCREEN_MAX_HEIGHT * 0.15 + ((level.fontHeight * 1.5) * self.menuCursor[self.menuKey]));
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

generateMenuOptions()
{
	self addMenu("main");
	is_host = self GetEntityNumber() == 0;

	// DVAR menu
	self addMenuOption("main", "DVAR Menu", ::menuAction, "CHANGE_MENU", "dvar_menu");
	self addMenu("dvar_menu", "main");
	self addMenuOption("dvar_menu", "^1Reset All^7", ::reset_all_client_dvars);
	dvars = getarraykeys(level.DVARS);
	for (i = dvars.size - 1; i >= 0; i--) // reverse order to display the dvars in the order they are defined
	{
		dvar = level.DVARS[dvars[i]];
		if (!is_host && isdefined(dvar.scope) && dvar.scope == "global")
			continue;
		if (dvar.type == "slider")
			self addMenuOption("dvar_menu", dvar.name, ::slider_start, dvar);
		else if (dvar.type == "boolean")
			self addMenuOption("dvar_menu", dvar.name, ::toggle_boolean_dvar, dvar);
	}

	// Host submenu
	if (is_host)
	{
		self addMenuOption("main", "Global settings", ::menuAction, "CHANGE_MENU", "host_menu");
		self addMenu("host_menu", "main");
		self addMenuOption("host_menu", "Toggle Old School Mode", ::toggleOldschool);

		// Map Menu
		if (getDvarInt("ui_allow_teamchange") == 1)
		{
			// Map selector
			self addMenuOption("main", "Select map", ::menuAction, "CHANGE_MENU", "host_menu_maps");
			self addMenu("host_menu_maps", "main");
			maps = getarraykeys(level.MAPS);
			// loop in reverse to display the maps in the order they are defined
			for (i = maps.size - 1; i >= 0; i--)
			{
				mapname = maps[i];
				label = level.MAPS[mapname];
				self addMenuOption("host_menu_maps", label, ::changeMap, mapname);
			}
		}
	}

	self addMenuOption("main", "Game Objects Menu", ::menuAction, "CHANGE_MENU", "menu_game_objects");

	self addMenu("menu_game_objects", "main");
	self addMenuOption("menu_game_objects", "Spawn Object", ::menuAction, "CHANGE_MENU", "menu_game_objects_spawn");
	self addMenu("menu_game_objects_spawn", "menu_game_objects");

	// create a submenu for each model type
	modelnames = getarraykeys(level.FORGE_MODELS);
	for (i = 0; i < modelnames.size; i++)
	{
		modelName = modelnames[i];
		count = level.FORGE_MODELS[modelName].size;
		if (count == 0) // skip empty model types
			continue;
		else if (count == 1) // if there is only one model of this type, don't create a submenu
		{
			modelEnt = level.FORGE_MODELS[modelName][0];
			self addMenuOption("menu_game_objects_spawn", modelName, ::spawnGameObject, modelEnt);
		}
		else
		{
			menuLabel = modelName + " " + " (" + count + ")";
			menuKey = "menu_game_objects_select_" + modelName;
			self addMenuOption("menu_game_objects_spawn", menuLabel, ::menuAction, "CHANGE_MENU", menuKey);
			self addMenu(menuKey, "menu_game_objects_spawn");

			for (j = 0; j < count; j++)
			{
				modelEnt = level.FORGE_MODELS[modelName][j];
				menuLabel = modelName + " " + (j + 1);
				self addMenuOption(menuKey, menuLabel, ::spawnGameObject, modelEnt);
			}
		}
	}

	if (is_host)
	{
		self addMenuOption("menu_game_objects", "Show/Hide Domination", ::show_hide_by_script_gameobjectname, "dom");
		self addMenuOption("menu_game_objects", "Show/Hide HQ", ::show_hide_by_script_gameobjectname, "hq");
		self addMenuOption("menu_game_objects", "Show/Hide Sab", ::show_hide_by_script_gameobjectname, "sab");
		self addMenuOption("menu_game_objects", "Show/Hide SD", ::show_hide_by_script_gameobjectname, "bombzone");
		self addMenuOption("menu_game_objects", "^1Reset All!^7", ::resetAllGameObjects);
	}

	self addMenuOption("main", "Loadout Menu", ::menuAction, "CHANGE_MENU", "loadout_menu");
	// Assault Rifles menu
	self addMenu("assault_rifles_menu", "loadout_menu");
	self addMenuOption("assault_rifles_menu", "AK47", ::replace_weapon, "ak47_mp");
	self addMenuOption("assault_rifles_menu", "G3", ::replace_weapon, "g3_mp");
	self addMenuOption("assault_rifles_menu", "G36C", ::replace_weapon, "g36c_mp");
	self addMenuOption("assault_rifles_menu", "M14", ::replace_weapon, "m14_mp");
	self addMenuOption("assault_rifles_menu", "M16A4", ::replace_weapon, "m16_mp");
	self addMenuOption("assault_rifles_menu", "M4A1", ::replace_weapon, "m4_mp");
	self addMenuOption("assault_rifles_menu", "MP44", ::replace_weapon, "mp44_mp");

	// LMGs menu
	self addMenu("lmgs_menu", "loadout_menu");
	self addMenuOption("lmgs_menu", "M249 SAW", ::replace_weapon, "saw_mp");
	self addMenuOption("lmgs_menu", "M60E4", ::replace_weapon, "m60e4_mp");
	self addMenuOption("lmgs_menu", "RPD", ::replace_weapon, "rpd_mp");

	// Pistols menu
	self addMenu("pistols_menu", "loadout_menu");
	self addMenuOption("pistols_menu", "Colt 45", ::replace_weapon, "colt45_mp");
	self addMenuOption("pistols_menu", "Desert Eagle", ::replace_weapon, "deserteagle_mp");
	self addMenuOption("pistols_menu", "Desert Eagle Gold", ::replace_weapon, "deserteaglegold_mp");
	self addMenuOption("pistols_menu", "M9 Beretta", ::replace_weapon, "beretta_mp");
	self addMenuOption("pistols_menu", "USP .45", ::replace_weapon, "usp_mp");

	// Shotguns menu
	self addMenu("shotguns_menu", "loadout_menu");
	self addMenuOption("shotguns_menu", "M1014", ::replace_weapon, "m1014_mp");
	self addMenuOption("shotguns_menu", "Winchester 1200", ::replace_weapon, "winchester1200_mp");

	// SMGs menu
	self addMenu("smgs_menu", "loadout_menu");
	self addMenuOption("smgs_menu", "AK74u", ::replace_weapon, "ak74u_mp");
	self addMenuOption("smgs_menu", "Mini-Uzi", ::replace_weapon, "uzi_mp");
	self addMenuOption("smgs_menu", "MP5", ::replace_weapon, "mp5_mp");
	self addMenuOption("smgs_menu", "P90", ::replace_weapon, "p90_mp");
	self addMenuOption("smgs_menu", "Skorpion", ::replace_weapon, "skorpion_mp");

	// Sniper Rifles menu
	self addMenu("sniper_rifles_menu", "loadout_menu");
	self addMenuOption("sniper_rifles_menu", "Barrett .50cal", ::replace_weapon, "barrett_mp");
	self addMenuOption("sniper_rifles_menu", "Dragunov", ::replace_weapon, "dragunov_mp");
	self addMenuOption("sniper_rifles_menu", "M21", ::replace_weapon, "m21_mp");
	self addMenuOption("sniper_rifles_menu", "M40A3", ::replace_weapon, "m40a3_mp");
	self addMenuOption("sniper_rifles_menu", "R700", ::replace_weapon, "remington700_mp");

	// Camo menu
	self addMenu("camo_menu", "loadout_menu");
	self addMenuOption("camo_menu", "None", ::give_camo, 0);
	self addMenuOption("camo_menu", "Desert", ::give_camo, 1);
	self addMenuOption("camo_menu", "Woodland", ::give_camo, 2);
	self addMenuOption("camo_menu", "Digital", ::give_camo, 3);
	self addMenuOption("camo_menu", "Blue Tiger", ::give_camo, 5);
	self addMenuOption("camo_menu", "Red Tiger", ::give_camo, 4);
	self addMenuOption("camo_menu", "Gold", ::give_camo, 6);

	// Loadout menu
	self addMenu("loadout_menu", "main");
	self addMenuOption("loadout_menu", "Assault Rifles", ::menuAction, "CHANGE_MENU", "assault_rifles_menu");
	self addMenuOption("loadout_menu", "LMGs", ::menuAction, "CHANGE_MENU", "lmgs_menu");
	self addMenuOption("loadout_menu", "Pistols", ::menuAction, "CHANGE_MENU", "pistols_menu");
	self addMenuOption("loadout_menu", "Shotguns", ::menuAction, "CHANGE_MENU", "shotguns_menu");
	self addMenuOption("loadout_menu", "SMGs", ::menuAction, "CHANGE_MENU", "smgs_menu");
	self addMenuOption("loadout_menu", "Sniper Rifles", ::menuAction, "CHANGE_MENU", "sniper_rifles_menu");
	self addMenuOption("loadout_menu", "Camo Menu", ::menuAction, "CHANGE_MENU", "camo_menu");
	self addMenuOption("loadout_menu", "Sleight of Hand", ::toggle_fast_reload);
	self addMenuOption("loadout_menu", "RPG Switch", ::toggleRPGSwitch);

	self addMenuOption("main", "Player Settings", ::menuAction, "CHANGE_MENU", "player_settings");
	self addMenu("player_settings", "main");
	self addMenuOption("player_settings", "Set Save Index", ::setSaveIndex);
	self addMenuOption("player_settings", "Distance HUD", ::toggle_hud_display, "distance");
	self addMenuOption("player_settings", "Speed HUD", ::toggle_hud_display, "speed");
	self addMenuOption("player_settings", "Height HUD", ::toggle_hud_display, "z_origin");

	self addMenuOption("player_settings", "Jump Crouch", ::toggleJumpCrouch);
	self addMenuOption("player_settings", "Lean Toggle", ::LeanBindToggle);
	self addMenuOption("player_settings", "Cycle Visions", ::CycleVision);
	self addMenuOption("player_settings", "Revert Vision", ::RevertVision);
	self addMenuOption("player_settings", "Look Straight Down", ::toggle_look_straight_down);

	// Bot submenu
	self addMenuOption("main", "Bot Menu", ::menuAction, "CHANGE_MENU", "bot_menu");
	self addMenu("bot_menu", "main");
	for (i = 0; i < self.cj["maxbots"]; i++)
	{
		text = "";
		if (self.cj["botnumber"] == i)
			text += level.SELECTED_PREFIX;

		text += "Set active bot " + (i + 1);
		// If bot is already spawned display its origin
		// useful to record good bot positions
		if (isplayer(self.cj["bots"][i]))
		{
			origin = self.cj["bots"][i].origin;
			origin = (int(origin[0]), int(origin[1]), int(origin[2]));
			text += (" " + origin);
		}

		self addMenuOption("bot_menu", text, ::setSelectedBot, i);
	}
	self addMenuOption("bot_menu", "Spawn Floating Bot", ::spawnFloatingBot);
	if (is_host)
		self addMenuOption("bot_menu", "Kick All Bots", ::kickAllBots);

	// Clone submenu
	self addMenuOption("main", "Clone Menu", ::menuAction, "CHANGE_MENU", "clone_menu");
	self addMenu("clone_menu", "main");
	self addMenuOption("clone_menu", "Spawn Clone", ::addClone);
	self addMenuOption("clone_menu", "Remove Clones", ::deleteClones);

	// Enhanced submenu
	if (is_host)
	{
		self addMenuOption("main", "Enhanced Menu", ::menuAction, "CHANGE_MENU", "enhanced_menu"); // Add to main menu

		self addMenu("enhanced_menu", "main");
		self addMenuOption("enhanced_menu", "Barrier Menu", ::menuAction, "CHANGE_MENU", "barrier_menu");
		self addMenuOption("enhanced_menu", "Bot Action Menu", ::menuAction, "CHANGE_MENU", "bot_action_menu");

		// Barrier submenu
		self addMenu("barrier_menu", "enhanced_menu");
		self addMenuOption("barrier_menu", "Remove All Barriers", ::removeBarriersOverHeight, 0);
		self addMenuOption("barrier_menu", "Remove Barriers > 100 Height", ::removeBarriersOverHeight, 100);
		self addMenuOption("barrier_menu", "Remove Barriers > 500 Height", ::removeBarriersOverHeight, 500);
		self addMenuOption("barrier_menu", "Remove Barriers > 1000 Height", ::removeBarriersOverHeight, 1000);
		self addMenuOption("barrier_menu", "Remove Barriers > 1500 Height", ::removeBarriersOverHeight, 1500);
		self addMenuOption("barrier_menu", "Enable Collision at origin", ::enablecollisionforbrushcontainingorigin_wrapper);
		self addMenuOption("barrier_menu", "Disable Collision at origin", ::disablecollisionforbrushcontainingorigin_wrapper);
		self addMenuOption("barrier_menu", "Restore Barriers", ::restoreBarriers);

		// Bot Action submenu
		self addMenu("bot_action_menu", "enhanced_menu");
		self addMenuOption("bot_action_menu", "Auto Mantle ON/OFF", ::toggleAutoMantle);
		self addMenuOption("bot_action_menu", "Trigger Distance UP", ::modifyTriggerDistance, 10);
		self addMenuOption("bot_action_menu", "Trigger Distance DOWN", ::modifyTriggerDistance, -10);
	}

	// Player model menu
	addMenuOption("main", "Player Model Menu", ::menuAction, "CHANGE_MENU", "player_model_menu");
	self addMenu("player_model_menu", "main");
	keys = getarraykeys(level.PLAYER_MODELS);
	for (i = keys.size - 1; i >= 0; i--) // reverse order to display the maps in the order they are defined
	{
		model = keys[i];
		model_friendly_name = level.PLAYER_MODELS[model];
		self addMenuOption("player_model_menu", model_friendly_name, ::change_player_model, model);
	}

	self addMenuOption("main", "Theme Menu", ::menuAction, "CHANGE_MENU", "theme_menu");
	self addMenu("theme_menu", "main");
	themes = getarraykeys(level.THEMES);
	for (i = themes.size - 1; i >= 0; i--) // reverse order to display in the order they are defined
		self addMenuOption("theme_menu", themes[i], ::menuAction, "CHANGE_THEME", themes[i]);
}

/**
 * Constantly replace the players ammo.
 */
replenish_ammo()
{
	self endon("end_respawn");
	self endon("disconnect");

	for (;;)
	{
		currentWeapon = self getCurrentWeapon(); // undefined if the player is mantling or on a ladder
		if (isdefined(currentWeapon))
			self giveMaxAmmo(currentWeapon);
		wait 1;
	}
}

watch_nightvision_press()
{
	self endon("disconnect");
	self endon("end_respawn");

	for (;;)
	{
		common_scripts\utility::waittill_any("night_vision_on", "night_vision_off");
		self.nightVisionButtonPressedTime = getTime();
	}
}

watch_buttons()
{
	self endon("disconnect");
	self endon("end_respawn");

	self thread watch_nightvision_press();

	for (;;)
	{
		if (!self.cj["menu_open"])
		{
			if (self button_pressed_twice("use"))
			{
				self thread menuAction("OPEN");
				wait .2;
			}
			else if (self button_pressed_twice("melee"))
			{
				self savePos(self.cj["savenum"]);
				wait .2;
			}
			else if (!self.ufo && self button_pressed("smoke"))
			{
				self loadPos(self.cj["savenum"]);
				wait .2;
			}
			else if (self button_pressed("frag"))
			{
				if (!self.ufo)
					self thread forgestart();
				else
					self ufoend();

				wait .2;
			}
			else if (self button_pressed("nightvision"))
			{
				self thread spawnSelectedBot();
				wait .2;
			}
		}
		else
		{
			if (self button_pressed("use"))
			{
				self menuAction("SELECT");
				wait .2;
			}
			else if (self button_pressed("melee"))
			{
				self menuAction("BACK");
				wait .2;
			}
			else if (self button_pressed("melee"))
			{
				self menuAction("CLOSE");
				wait .2;
			}
			else if (self button_pressed("ads"))
			{
				self menuAction("UP");
				wait .2;
			}
			else if (self button_pressed("attack"))
			{
				self menuAction("DOWN");
				wait .2;
			}
		}
		wait .05;
	}
}

savePos(i)
{
	if (!self isOnGround())
		return;

	self.cj["settings"]["rpg_switched"] = false;
	self.cj["saves"]["org"][i] = self.origin;
	self.cj["saves"]["ang"][i] = self getPlayerAngles();
}

loadPos(i)
{
	self freezecontrols(true);
	wait 0.05;

	self setPlayerAngles(self.cj["saves"]["ang"][i]);
	self setOrigin(self.cj["saves"]["org"][i]);

	self notify("position_loaded");

	// pull out rpg on load if RPG switch is enabled
	if (self.cj["settings"]["rpg_switch_enabled"] && self.cj["settings"]["rpg_switched"])
	{
		self switchToWeapon("rpg_mp");
		self.cj["settings"]["rpg_switched"] = false;
	}

	wait 0.05;
	self freezecontrols(false);
}

initBot()
{
	bot = addtestclient();

	if (!isDefined(bot))
		return undefined;

	bot.pers["isBot"] = true;

	while (!isDefined(bot.pers["team"]))
		wait 0.05;

	bot [[level.axis]] ();

	wait 0.5;

	bot.class = level.defaultClass;
	bot.pers["class"] = level.defaultClass;
	bot [[level.spawnClient]] ();

	wait .1;

	// plugin handles bot controls
	bot freezeControls(false);

	return bot;
}

setSelectedBot(num)
{
	self.cj["botnumber"] = num;
	self iPrintLn("Bot " + (num + 1) + " active. Press [{+actionslot 1}] to update position.");
}

spawnSelectedBot()
{
	if (!isdefined(self.cj["bots"][self.cj["botnumber"]]))
	{
		self.cj["bots"][self.cj["botnumber"]] = initBot();
		if (!isdefined(self.cj["bots"][self.cj["botnumber"]]))
		{
			self iPrintLn("^1Couldn't spawn a bot");
			return;
		}
	}

	origin = self.origin;
	playerAngles = self getPlayerAngles();

	wait 0.5;
	for (i = 3; i > 0; i--)
	{
		self iPrintLn("Bot spawns in ^2" + i);
		wait 1;
	}
	self.cj["bots"][self.cj["botnumber"]] setOrigin(origin);
	// Face the bot the same direction the player was facing
	self.cj["bots"][self.cj["botnumber"]] setPlayerAngles((0, playerAngles[1], 0));
	self.cj["bots"][self.cj["botnumber"]] savePos(0); // Save the bot's position for auto mantle
}

toggleOldschool()
{
	setting = "oldschool";
	printName = "Old School Mode";

	if (!isdefined(self.cj["settings"][setting]) || self.cj["settings"][setting] == false)
	{
		self.cj["settings"][setting] = true;
		self.cj["settings"]["jump_slowdownEnable"] = false;
		setDvar("jump_height", 64);
		setDvar("jump_slowdownEnable", 0);
		iPrintln(printName + " [^2ON^7]");
	}
	else
	{
		self.cj["settings"][setting] = false;
		self.cj["settings"]["jump_slowdownEnable"] = true;
		setDvar("jump_height", 39);
		setDvar("jump_slowdownEnable", 1);
		iPrintln(printName + " [^1OFF^7]");
	}
}

addClone()
{
	body = self clonePlayer(100000);
	self.cj["clones"][self.cj["clones"].size] = body;
}

changeMap(mapname)
{
	Map(mapname);
}

deleteClones()
{
	clones = self.cj["clones"];

	for (i = 0; i < clones.size; i++)
		clones[i] delete ();
}

spawnFloatingBot()
{
	bot = initBot();
	origin = self.origin;
	playerAngles = self getPlayerAngles();

	for (i = 3; i > 0; i--)
	{
		self iPrintLn("Floating bot spawns in ^2" + i);
		wait 1;
	}

	bot setOrigin(origin);
	// Face the bot the same direction the player was facing
	bot setPlayerAngles((0, playerAngles[1], 0));

	self.floating_bot = spawn("script_origin", self.origin);
	bot linkto(self.floating_bot);
}

toggleAutoMantle()
{
	if (!isdefined(self.cj["settings"]["automantle"]) || self.cj["settings"]["automantle"] == false)
	{
		self.cj["settings"]["automantle"] = true;
		self iprintln("Auto Mantle [^2ON^7]");
		self thread startAutoMantle();
	}
	else
	{
		self.cj["settings"]["automantle"] = false;
		self iprintln("Auto Mantle [^1OFF^7]");
		self stopAutoMantle();
	}
}

modifyTriggerDistance(value)
{
	if (!isdefined(self.triggerDistance))
		self.triggerDistance = 200;

	self.triggerDistance += value;
	self iprintln("Trigger distance: " + self.triggerDistance);
}

startAutoMantle()
{
	self endon("disconnect");
	self endon("death");
	self endon("stop_automantle");

	if (!isdefined(self.triggerDistance))
		self.triggerDistance = 200;

	bot = self.cj["bots"][self.cj["botnumber"]];
	if (!isdefined(bot))
	{
		self iprintln("Could not find bot" + self.cj["botnumber"]);
		self.cj["settings"]["automantle"] = false;
		return;
	}
	else
	{
		self iprintln("Watching player: " + bot.name);
		self iprintln("Trigger distance: " + self.triggerDistance);
	}

	bot savePos(0);
	botEye = bot getEye();

	for (;;)
	{
		if (distance(botEye, self getorigin()) < self.triggerDistance)
		{
			bot botjump();
			self waittill("position_loaded");
			// wait for bot to finish mantling before loading position
			if (bot ismantling())
				wait 0.5;

			bot loadPos(0);
		}
		wait 0.05;
	}
}

stopAutoMantle()
{
	self notify("stop_automantle");
	self iprintln("Stopped automantle");
}
