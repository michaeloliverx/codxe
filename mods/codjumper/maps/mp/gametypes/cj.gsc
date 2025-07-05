#include common_scripts\utility;
#include maps\mp\gametypes\_hud_util;


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

		player setupPlayer();
		player onPlayerConnectDvars();
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
		self onPlayerSpawnedDvars();
	}
}

onPlayerConnectDvars()
{
	// JumpCrouch / binds helper
	self setClientDvar("activeaction", "vstr VSTR_LEAN_DISABLED;");
	self setClientDvar("VSTR_LEAN_ENABLED", "bind BUTTON_A vstr BUTTON_A_ACTION;bind DPAD_DOWN +actionslot 3; bind DPAD_LEFT +leanleft; bind DPAD_RIGHT +leanright");
	self setClientDvar("VSTR_LEAN_DISABLED", "bind BUTTON_A vstr BUTTON_A_ACTION;bind DPAD_DOWN +actionslot 2; bind DPAD_LEFT +actionslot 3; bind DPAD_RIGHT +actionslot 4");
	self setClientDvar("BUTTON_A_ACTION", "+gostand;-gostand");

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

	self setClientDvar("cg_scoreboardPingText", 1);

	self setClientDvar("cg_chatHeight", 0); // prevent people from freezing consoles via say command

	// look straight up
	self setclientdvar("player_view_pitch_up", 89.9);

	// Remove glow color applied to the mode and map name strings on the connect screen
	self setClientDvar("ui_ConnectScreenTextGlowColor", 0);

	self setClientDvar("cg_descriptiveText", 0);		  // Remove spectator button icons and text
	self setClientDvar("player_spectateSpeedScale", 1.5); // Faster movement in spectator/ufo
}

onPlayerSpawnedDvars()
{
	// If a player has a custom FOV set, use it
	if (isdefined(self.cj["settings"]["cg_fov"]))
		self setClientDvar("cg_fov", self.cj["settings"]["cg_fov"]);

	// HUD
	self setClientDvar("ui_hud_hardcore", 1);		// Hardcore HUD
	self setClientDvar("cg_drawCrosshair", 0);		// Disable crosshair
	self setClientDvar("g_compassShowEnemies", 0);	// Disable compass
}

is_int(num)
{
	return num == int(num);
}

to_int_vector(origin)
{
	return (int(origin[0]), int(origin[1]), int(origin[2]));
}

toggle_hud_display(type)
{
	if (!isdefined(self.cj["meter_hud"]))
		self.cj["meter_hud"] = [];

	// not defined means OFF
	if (!isdefined(self.cj["meter_hud"][type]))
	{
		if (type == "distance")
			self thread start_hud_distance();
		else if (type == "speed")
			self thread start_hud_speed();
		else if (type == "z_origin")
			self thread start_hud_distance_z_origin();
	}
	else
	{
		self notify("end_hud_" + type);
		self.cj["meter_hud"][type] destroy();
	}
}

start_hud_distance()
{
	self endon("disconnect");
	self endon("end_hud_distance");

	fontScale = 1.4;
	x = 62;
	y = 10;

	self.cj["meter_hud"]["distance"] = createFontString("small", fontScale);
	self.cj["meter_hud"]["distance"] setPoint("BOTTOMRIGHT", "BOTTOMRIGHT", x, y);
	self.cj["meter_hud"]["distance"].alpha = 0.5;
	self.cj["meter_hud"]["distance"].label = &"distance:&&1";

	for (;;)
	{
		// trace using the player's eye position
		// but measure distance from the player's origin
		angles = self getPlayerAngles();
		origin = self.origin;

		stance = self getStance();
		if (stance == "prone")
			eye = self.origin + (0, 0, 11);
		else if (stance == "crouch")
			eye = self.origin + (0, 0, 40);
		else
			eye = self.origin + (0, 0, 60);

		start = eye;
		end = start + maps\mp\_utility::vector_scale(anglestoforward(angles), 999999);

		endpos = PhysicsTrace(start, end);

		distance = distance(origin, endpos);
		self.cj["meter_hud"]["distance"] setValue(distance);

		wait 0.05;
	}
}

start_hud_speed()
{
	self endon("disconnect");
	self endon("end_hud_speed");

	fontScale = 1.4;
	x = 62;
	y = 22;
	alpha = 0.5;

	self.cj["meter_hud"]["speed"] = createFontString("small", fontScale);
	self.cj["meter_hud"]["speed"] setPoint("BOTTOMRIGHT", "BOTTOMRIGHT", x, y);
	self.cj["meter_hud"]["speed"].alpha = alpha;
	self.cj["meter_hud"]["speed"].label = &"speed:&&1";

	for (;;)
	{
		velocity3D = self getVelocity();
		horizontalSpeed2D = int(sqrt(velocity3D[0] * velocity3D[0] + velocity3D[1] * velocity3D[1]));
		self.cj["meter_hud"]["speed"] setValue(horizontalSpeed2D);

		wait 0.05;
	}
}

start_hud_distance_z_origin()
{
	self endon("disconnect");
	self endon("end_hud_z_origin");

	fontScale = 1.4;
	x = 62;
	y = 36;

	self.cj["meter_hud"]["z_origin"] = createFontString("small", fontScale);
	self.cj["meter_hud"]["z_origin"] setPoint("BOTTOMRIGHT", "BOTTOMRIGHT", x, y);
	self.cj["meter_hud"]["z_origin"].alpha = 0.5;
	self.cj["meter_hud"]["z_origin"].label = &"z:&&1";

	for (;;)
	{
		self.cj["meter_hud"]["z_origin"] setValue(self.origin[2]);

		wait 0.05;
	}
}

toggleJumpCrouch()
{
	setting = "jumpcrouch_enabled";
	printName = "Jump Crouch";

	if (!isdefined(self.cj["settings"][setting]) || self.cj["settings"][setting] == false)
	{
		self.cj["settings"][setting] = true;
		self setClientDvar("BUTTON_A_ACTION", "+gostand;-gostand;wait 4;togglecrouch");
		self iPrintln(printName + " [^2ON^7]");
	}
	else
	{
		self.cj["settings"][setting] = false;
		self setClientDvar("BUTTON_A_ACTION", "+gostand;-gostand");
		self iPrintln(printName + " [^1OFF^7]");
	}
}

toggleRPGSwitch()
{
	setting = "rpg_switch_enabled";
	printName = "RPG Switch";

	if (!isdefined(self.cj["settings"][setting]) || self.cj["settings"][setting] == false)
	{
		self.cj["settings"][setting] = true;
		self thread rpgSwitch();
		self iPrintln(printName + " [^2ON^7]");
	}
	else
	{
		self.cj["settings"][setting] = false;
		self notify("stop_rpg_switch");
		self iPrintln(printName + " [^1OFF^7]");
	}
}

rpgSwitch()
{
	self endon("disconnect");
	self endon("end_respawn");

	self notify("stop_rpg_switch");
	self endon("stop_rpg_switch");

	while(self.cj["settings"]["rpg_switch_enabled"])
	{
		self waittill("weapon_fired");
		if (self getCurrentWeapon() == "rpg_mp")
		{
			self.cj["settings"]["rpg_switched"] = true;
			self switchToWeapon(self.cj["loadout"].sidearm);
			wait 0.4;
			self SetWeaponAmmoClip("rpg_mp", 1);
		}
	}
}

toggle_pc_lookdown_pitch()
{
	console_default = 70;
	pc_default = 85;

	if(getDvarInt("player_view_pitch_down") == console_default)
	{
		setdvar("player_view_pitch_down", pc_default);
		iPrintln("PC lookdown pitch [^2ON^7]");
	}
	else
	{
		setdvar("player_view_pitch_down", console_default);
		iPrintln("PC lookdown pitch [^1OFF^7]");
	}
}

resetAllGameObjects()
{
	modelnames = getarraykeys(level.FORGE_MODELS);
	for (i = 0; i < modelnames.size; i++)
	{
		modelName = modelnames[i];
		for (j = 0; j < level.FORGE_MODELS[modelName].size; j++)
		{
			modelEnt = level.FORGE_MODELS[modelName][j];
			modelEnt.origin = modelEnt.startOrigin;
			modelEnt.angles = modelEnt.startAngles;
		}
	}
	iprintln("All game objects reset");
}

show_hide_by_script_gameobjectname(script_gameobjectname)
{
	hidden = false;
	ents = getentarray();
	for (i = 0; i < ents.size; i++)
	{
		if (isdefined(ents[i].script_gameobjectname) && ents[i].script_gameobjectname == script_gameobjectname)
		{
			if(isdefined(ents[i].hidden) && ents[i].hidden)
			{
				ents[i] show();
				ents[i] solid();
				ents[i].hidden = false;
			}
			else
			{
				ents[i] hide();
				ents[i] notsolid();
				ents[i].hidden = true;
				hidden = true;
			}
		}
	}

	action = "shown";
	if(hidden)
		action = "hidden";

	type = script_gameobjectname;
	if(type == "bombzone")
		type = "sd";

	iprintln(type + " " + action);
}

spawnGameObject(ent)
{
	playerAngles = self getPlayerAngles();
	ent.origin = to_int_vector(self.origin + (anglestoforward(playerAngles) * 150));
	ent.angles = to_int_vector((0, playerAngles[1], 0));
	self iprintln("Object spawned at " + ent.origin + ent.angles);
}

enableLeanBinds()
{
	self setClientDvar("activeaction", "vstr VSTR_LEAN_ENABLED");
	self iPrintln("Lean Binds [^2ON^7] (Requires map restart)");
	self iPrintln("[{+actionslot 3}]/[{+actionslot 4}] for lean and [{+actionslot 2}] for RPG");
}

disableLeanBinds()
{
	self setClientDvar("activeaction", "vstr VSTR_LEAN_DISABLED");
	self iPrintln("Lean Binds [^1OFF^7] (Requires map restart)");
}

LeanBindToggle()
{
    if (!isDefined(self.LeanBind) || self.LeanBind == false) 
    {
        self.LeanBind = true;
    }
    else 
    {
        self.LeanBind = false;
    }
    if (self.LeanBind == true)
    {
        enableLeanBinds();
    }
    else if (self.LeanBind == false)
    {
        disableLeanBinds();
    }
}
RevertVision()
{
	VisionSetNaked( getDvar( "mapname" ), 3.0 );
	self.CVIndex = 0;
}
CycleVision()
{
	level.visionModes[0] = "blank";
	level.visionModes[1] = "cheat_chaplinnight";
	level.visionModes[2] = "aftermath";
	level.visionModes[3] = "default_night";	
	level.visionModes[4] = "mp_convoy";
   	level.visionModes[5] = "mp_bloc";
	level.visionModes[6] = "mp_backlot";
    	level.visionModes[7] = "mp_bog";
    	level.visionModes[8] = "mp_crash";
	level.visionModes[9] = "mp_citystreets";
    	level.visionModes[10] = "mp_crossfire";
	level.visionModes[11] = "mp_farm";
    	level.visionModes[12] = "mp_vacant";
    	level.visionModes[13] = "mp_overgrown";
    	level.visionModes[14] = "mp_pipeline";
	level.visionModes[15] = "mp_shipment";
    	level.visionModes[16] = "mp_showdown";
    	level.visionModes[17] = "mp_strike";
    	level.visionModes[18] = "mp_countdown";
   	level.visionModes[19] = "mp_cargoship";
    if (!isDefined(self.CVIndex))
    {
        self.CVIndex = 0;
    }
    if (self.CVIndex >= level.visionModes.size)
    {
        self.CVIndex = 0; 
    }
    VisionSetNaked(level.visionModes[self.CVIndex], 1.5);
    self iPrintlnBold("Vision Mode: ^3" + level.visionModes[self.CVIndex]);
    self.CVIndex++;
}

kickAllBots()
{
	for ( i = 0; i < level.players.size; i++ )
		if ( isdefined(level.players[i].pers["isBot"]) )
			kick(level.players[i] getEntityNumber());
}

getPlayerFromName(playerName)
{
	for (i = 0; i < level.players.size; i++)
		if (level.players[i].name == playerName)
			return level.players[i];
}

getForgeInstructionsText(state)
{
	instructions = [];

	if (!isdefined(state))
	{
		instructions[instructions.size] = "[{+activate}] Hold for more options";
		instructions[instructions.size] = "[{+frag}] Exit";
	}
	else if (state == "FOCUSED")
	{
		instructions[instructions.size] = "[{+activate}] Hold for more options";
		instructions[instructions.size] = "[{+smoke}] Decrease";
		instructions[instructions.size] = "[{+frag}] Increase";
	}
	else if (state == "HOLD_X")
	{
		instructions[instructions.size] = "[{+smoke}] Prev mode";
		instructions[instructions.size] = "[{+frag}] Next mode";

		instructions[instructions.size] = "[{+speed_throw}] Exit Forge";
		instructions[instructions.size] = "[{+attack}] Pick up/Drop";
		if (level.xenon)
			instructions[instructions.size] = "[{+breath_sprint}] Clone object";

		instructions[instructions.size] = "[{+melee}] Switch to UFO mode";
	}

	instructionsString = "";
	for (i = 0; i < instructions.size; i++)
		instructionsString += instructions[i] + "\n";

	return instructionsString;
}

createforgehud()
{
	self.forge_hud = [];
	self.forge_hud["instructions"] = createFontString("default", 1.4);
	self.forge_hud["instructions"] setPoint("TOPLEFT", "TOPLEFT", -30, -20);
	self.forge_hud["instructions"] setText(getForgeInstructionsText());

	x = 30;

	self.forge_hud["entities"] = createFontString("default", 1.4);
	self.forge_hud["entities"] setPoint("TOPRIGHT", "TOPRIGHT", x, -20);
	self.forge_hud["entities"].label = &"entities (1000 max): &&1";
	self.forge_hud["entities"] SetValue(getentarray().size);

	self.forge_hud["mode"] = createFontString("default", 1.4);
	self.forge_hud["mode"] setPoint("TOPRIGHT", "TOPRIGHT", x, 0);
	self.forge_hud["mode"] setText("mode: " + self forge_get_mode());

	self.forge_hud["pitch"] = createFontString("default", 1.4);
	self.forge_hud["pitch"] setPoint("TOPRIGHT", "TOPRIGHT", x, 20);
	self.forge_hud["pitch"].label = &"pitch: &&1";
	self.forge_hud["pitch"] SetValue(0);
	self.forge_hud["pitch"].alpha = 0;

	self.forge_hud["yaw"] = createFontString("default", 1.4);
	self.forge_hud["yaw"] setPoint("TOPRIGHT", "TOPRIGHT", x, 40);
	self.forge_hud["yaw"].label = &"yaw: &&1";
	self.forge_hud["yaw"] SetValue(0);
	self.forge_hud["yaw"].alpha = 0;

	self.forge_hud["roll"] = createFontString("default", 1.4);
	self.forge_hud["roll"] setPoint("TOPRIGHT", "TOPRIGHT", x, 60);
	self.forge_hud["roll"].label = &"roll: &&1";
	self.forge_hud["roll"] SetValue(0);
	self.forge_hud["roll"].alpha = 0;

	self.forge_hud["x"] = createFontString("default", 1.4);
	self.forge_hud["x"] setPoint("TOPRIGHT", "TOPRIGHT", x, 80);
	self.forge_hud["x"].label = &"x: &&1";
	self.forge_hud["x"] SetValue(0);
	self.forge_hud["x"].alpha = 0;

	self.forge_hud["y"] = createFontString("default", 1.4);
	self.forge_hud["y"] setPoint("TOPRIGHT", "TOPRIGHT", x, 100);
	self.forge_hud["y"].label = &"y: &&1";
	self.forge_hud["y"] SetValue(0);
	self.forge_hud["y"].alpha = 0;

	self.forge_hud["z"] = createFontString("default", 1.4);
	self.forge_hud["z"] setPoint("TOPRIGHT", "TOPRIGHT", x, 120);
	self.forge_hud["z"].label = &"z: &&1";
	self.forge_hud["z"] SetValue(0);
	self.forge_hud["z"].alpha = 0;

	self.forge_hud["reticle"] = createIcon("reticle_flechette", 40, 40);
	self.forge_hud["reticle"] setPoint("center", "center", "center", "center");

	self waittill_any("end_respawn", "disconnect", "forge_end");
	self destroyforgehud();
}

destroyforgehud()
{
	huds = getarraykeys(self.forge_hud);
	for (i = 0; i < huds.size; i++)
		if (isdefined(self.forge_hud[huds[i]]))
			self.forge_hud[huds[i]] destroy();
}

forgestart()
{
	self endon("disconnect");
	self endon("end_respawn");
	self endon("forge_end");

	self ufocontrolsON();

	spectator_speed_settings = [];
	spectator_speed_settings["slowest"] = 0.1;
	spectator_speed_settings["slower"] = 0.25;
	spectator_speed_settings["slow"] = 0.5;
	spectator_speed_settings["normal"] = 1;
	spectator_speed_settings["fast"] = 1.5;
	spectator_speed_settings["faster"] = 3;

	if (!isdefined(self.spectator_speed_index))
		self.spectator_speed_index = 3;

	if (!isdefined(self.spectator_mode))
		self.spectator_mode = "ufo";

	if (self.spectator_mode == "forge")
		self thread createforgehud();

	if (self.spectator_mode == "ufo")
		self iprintln("UFO mode ON");
	else
		self iprintln("Forge mode ON");

	unfocusedColor = (0.5, 0.5, 0.5); // gray for unfocused
	focusedColor = (0, 1, 0);		  // green for focused
	pickedUpColor = (1, 0, 0);		  // red for picked up

	self.focusedEnt = undefined;
	self.pickedUpEnt = undefined;

	unit = 1;

	for (;;)
	{
		// prevent monitoring when in menu
		if (self.cj["menu_open"])
		{
			wait 0.1;
			continue;
		}

		// don't unfreeze controls if in menu otherwise the menu controls will break
		if (!self.cj["menu_open"])
			self freezecontrols(false);

		// HOLD X actions
		while (self usebuttonpressed())
		{
			self.forge_hud["instructions"] setText(getForgeInstructionsText("HOLD_X"));
			// freeze controls to allow meleebuttonpressed while in spectator
			self freezecontrols(true);
			if (self meleebuttonpressed())
			{
				if (self.spectator_mode == "ufo")
				{
					self thread createforgehud();
					self.spectator_mode = "forge";
					self setClientDvar("player_spectateSpeedScale", 0.5); // Slower speed for fine movements
					self iprintln("Forge mode");
					wait 0.25;
					break;
				}
				else
				{
					if (isdefined(self.pickedUpEnt))
					{
						self iprintln("Can't switch to UFO while holding an object");
						wait 0.1;
					}
					else
					{
						self thread destroyforgehud();
						self.spectator_mode = "ufo";
						self setClientDvar("player_spectateSpeedScale", 1.5);
						self iprintln("UFO mode");
						wait 0.25;
						break;
					}
				}
				wait 0.05;
			}

			if (self.spectator_mode == "forge") // Forge specific actions
			{
				// CLONE OBJECT
				if (self holdbreathbuttonpressed())
				{
					if (isdefined(self.pickedUpEnt))
					{
						self iprintln("Can't clone while holding an object");
						wait 0.1;
					}
					else if (isdefined(self.focusedEnt))
					{
						cloned_object = self cloneObject(self.focusedEnt);
						if (isdefined(cloned_object))
						{
							cloned_object linkto(self);
							self.pickedUpEnt = cloned_object;
							self.focusedEnt = cloned_object; // so HUD updates correctly
							self iprintln("Cloned and picked up " + getdisplayname(cloned_object));
							wait 0.25;
						}
						else
						{
							self iprintln("Can't clone " + getdisplayname(self.focusedEnt));
							wait 0.1;
						}
					}
				}
				// pick up or drop ent
				if (!isdefined(self.pickedUpEnt) && isdefined(self.focusedEnt) && self attackButtonPressed())
				{
					ent = self.focusedEnt;
					ent linkto(self);
					self.pickedUpEnt = self.focusedEnt;
					self iprintln("Picked up " + getdisplayname(ent));
					wait 0.25;
					break;
				}
				else if (isdefined(self.pickedUpEnt) && self attackButtonPressed())
				{
					ent = self.pickedUpEnt;
					ent unlink();
					ent.origin = to_int_vector(ent.origin); // snap to whole numbers
					self.pickedUpEnt = undefined;
					self iprintln("Dropped " + getdisplayname(ent));
					wait 0.25;
					break;
				}

				if (self fragbuttonpressed() || self secondaryoffhandbuttonpressed())
				{
					if (self fragbuttonpressed())
						self forge_change_mode("next");
					else if (self secondaryoffhandbuttonpressed())
						self forge_change_mode("prev");
					self.forge_hud["mode"] setText("mode: " + self forge_get_mode());
					wait 0.1;
				}
			}
			else if (self.spectator_mode == "ufo") // UFO specific actions
			{
				if (self secondaryoffhandbuttonpressed())
					self setplayerangles(self getPlayerAngles() - (0, 0, 1));
				else if (self fragbuttonpressed())
					self setplayerangles(self getPlayerAngles() + (0, 0, 1));
			}

			wait 0.05;
		}

		if (self.spectator_mode == "forge")
		{
			if (!isdefined(self.pickedUpEnt))
			{
				forward = anglestoforward(self getplayerangles());

				stance = self getStance();
				if (stance == "prone")
					eye = self.origin + (0, 0, 11);
				else if (stance == "crouch")
					eye = self.origin + (0, 0, 40);
				else
					eye = self.origin + (0, 0, 60);

				start = eye;
				end = vectorscale(forward, 9999);
				trace = bullettrace(start, start + end, true, self);
				if (isdefined(trace["entity"]))
				{
					ent = trace["entity"];
					self.forge_hud["reticle"].color = focusedColor;
					if (isdefined(ent.forge_parent))
						ent = ent.forge_parent;

					self.focusedEnt = ent;
				}
				else
				{
					self.forge_hud["reticle"].color = unfocusedColor;
					self.focusedEnt = undefined;
				}
			}
			else
			{
				self.forge_hud["reticle"].color = pickedUpColor;
			}

			// update hud

			if (isdefined(self.focusedEnt))
				self.forge_hud["instructions"] setText(getForgeInstructionsText("FOCUSED"));
			else
				self.forge_hud["instructions"] setText(getForgeInstructionsText());

			self.forge_hud["entities"] SetValue(getentarray().size);

			if (isdefined(self.focusedEnt))
			{
				self.forge_hud["pitch"] SetValue(self.focusedEnt.angles[0]);
				self.forge_hud["yaw"] SetValue(self.focusedEnt.angles[1]);
				self.forge_hud["roll"] SetValue(self.focusedEnt.angles[2]);
				self.forge_hud["x"] SetValue(self.focusedEnt.origin[0]);
				self.forge_hud["y"] SetValue(self.focusedEnt.origin[1]);
				self.forge_hud["z"] SetValue(self.focusedEnt.origin[2]);
				self.forge_hud["pitch"].alpha = 1;
				self.forge_hud["yaw"].alpha = 1;
				self.forge_hud["roll"].alpha = 1;
				self.forge_hud["x"].alpha = 1;
				self.forge_hud["y"].alpha = 1;
				self.forge_hud["z"].alpha = 1;
			}
			else
			{
				self.forge_hud["pitch"].alpha = 0;
				self.forge_hud["yaw"].alpha = 0;
				self.forge_hud["roll"].alpha = 0;
				self.forge_hud["x"].alpha = 0;
				self.forge_hud["y"].alpha = 0;
				self.forge_hud["z"].alpha = 0;
			}

			// rotations and movements can't be done on a linked entity so do it on focus
			if (!isdefined(self.pickedUpEnt) && isdefined(self.focusedEnt) && (self secondaryoffhandbuttonpressed() || self fragbuttonpressed()))
			{
				if (self secondaryoffhandbuttonpressed())
					self transform_object(self.focusedEnt, self forge_get_mode(), unit, 0.05);
				else if (self fragbuttonpressed())
					self transform_object(self.focusedEnt, self forge_get_mode(), unit * -1, 0.05);
			}
		}

		wait 0.05;
	}
}

// TODO: refactor forge mode into readable functions
// maybe switch statement with actions
// Add a way to delete entities
// Add a x,y movement mode
// add better datastructure for forge models, cloned objects don't inherit classname and targetnames etc

cloneObject(ent)
{
	if (!isdefined(ent))
	{
		self iprintln("No object to clone");
		return;
	}
	if (ent.classname != "script_brushmodel" && ent.classname != "script_model")
	{
		self iprintln("Entity classname must be one of {script_brushmodel, script_model}");
		return;
	}
	if (!isdefined(ent.forge_enabled) || !ent.forge_enabled)
	{
		self iprintln("Entity must be forge enabled");
		return;
	}
	if (getentarray().size >= 1000)
	{
		self iprintln("Max entities reached");
		return;
	}
	// TODO: maybe add a well known key to entity to indicate the forge type and its required properties to clone
	// case 1: 1 script_model, 1 script_brushmodel
	if (ent.classname == "script_model" && isdefined(ent.script_brushmodel))
	{
		script_model = spawn("script_model", ent.origin);
		script_model setmodel(ent.model);
		script_model.angles = ent.angles;

		script_brushmodel = spawn("script_model", ent.script_brushmodel.origin);
		script_brushmodel.angles = ent.script_brushmodel.angles;
		script_brushmodel clonebrushmodeltoscriptmodel(ent.script_brushmodel);
		script_brushmodel.classname = "script_brushmodel";
		script_brushmodel linkto(script_model);

		script_model.script_brushmodel = script_brushmodel;
		script_model.forge_enabled = true;

		return script_model;
	}
	// case 2: 1 script_brushmodel or script_model
	else if (!isdefined(ent.forge_children))
	{
		script_brushmodel = spawn("script_model", ent.origin);
		script_brushmodel.angles = ent.angles;
		script_brushmodel clonebrushmodeltoscriptmodel(ent);
		script_brushmodel.forge_enabled = true;

		return script_brushmodel;
	}
	// case 3: 1 script_brushmodel or script_model, multiple script_models
	else if (isdefined(ent.forge_children))
	{
		if (ent.forge_children.size == 0)
		{
			self iprintln("No forge_children to clone");
			return;
		}

		script_brushmodel = spawn("script_model", ent.origin);
		script_brushmodel.angles = ent.angles;
		script_brushmodel clonebrushmodeltoscriptmodel(ent);

		script_models = [];

		for (i = 0; i < ent.forge_children.size; i++)
		{
			script_model = spawn("script_model", ent.forge_children[i].origin);
			script_model setmodel(ent.forge_children[i].model);
			script_model.angles = ent.forge_children[i].angles;
			script_model linkto(script_brushmodel);
			script_model.forge_enabled = true;
			script_model.forge_parent = script_brushmodel;
			script_models[script_models.size] = script_model;
		}

		script_brushmodel.forge_enabled = true;
		script_brushmodel.forge_children = script_models;

		return script_brushmodel;
	}
}

ufoend()
{
	if (self.spectator_mode == "ufo")
	{
		self ufocontrolsOFF();
		self notify("forge_end");
		self iprintln("UFO mode OFF");
	}
	else if (self.spectator_mode == "forge")
	{
		if (isdefined(self.focusedEnt))
			return;
		if (isdefined(self.pickedUpEnt))
			self iprintln("Can't exit while holding an object");
		else
		{
			self notify("forge_end");
			self thread destroyforgehud();
			self ufocontrolsOFF();
			self freezecontrols(false);
			self iprintln("Forge mode OFF");
		}
	}
}

ufocontrolsON()
{
	self.ufo = true;
}

ufocontrolsOFF()
{
	self.ufo = false;
	self freezeControls(false);
}

getdisplayname(ent)
{
	if (isplayer(ent))
		return ent.name;
	else if (isdefined(ent.model) && ent.model != "")
		return ent.model;
	else
		return ent.classname;
}

forge_get_mode()
{
	return level.forge_change_modes[self.cj["forge_change_mode_index"]];
}

forge_change_mode(action)
{
	index = self.cj["forge_change_mode_index"];
	if (action == "next")
	{
		index += 1;
		if (index >= level.forge_change_modes.size)
			index = 0;
	}
	else if (action == "prev")
	{
		index -= 1;
		if (index < 0)
			index = level.forge_change_modes.size - 1;
	}

	self.cj["forge_change_mode_index"] = index;
}

transform_object(ent, axis, amount, time)
{
	if (!isdefined(ent))
	{
		self iprintln("No object to transform");
		return;
	}

	if(!is_int(ent.origin[0]) || !is_int(ent.origin[1]) || !is_int(ent.origin[2]))
		ent.origin = to_int_vector(ent.origin); // snap to whole numbers

	if(!is_int(ent.angles[0]) || !is_int(ent.angles[1]) || !is_int(ent.angles[2]))
		ent.angles = to_int_vector(ent.angles); // snap to whole numbers

	if (axis == "pitch")
		ent rotatepitch(amount, time);
	else if (axis == "yaw")
		ent rotateyaw(amount, time);
	else if (axis == "roll")
		ent rotateroll(amount, time);
	else if (axis == "x")
		ent moveX(amount, time);
	else if (axis == "y")
		ent moveY(amount, time);
	else if (axis == "z")
		ent moveZ(amount, time);
}


/**
 * Sets the origin of all spawnpoint entities to the specified origin.
 */
setAllSpawnPointsToOrigin(origin)
{
	ents = getentarray();
	for (i = 0; i < ents.size; i++)
		if (issubstr(ents[i].classname, "_spawn") && isdefined(ents[i].origin))
			ents[i].origin = origin;
}

/**
 * Delete all map entities that are no use for CJ to free up entity slots.
 */
deleteUselessEntities()
{
	// List of classnames to delete, 1 to delete, 0 to keep
	classnames_delete = [];
	classnames_delete["info_player_start"] = 1;
	classnames_delete["misc_mg42"] = 1;
	classnames_delete["misc_turret"] = 1;
	classnames_delete["mp_ctf_spawn_allies"] = 1;
	classnames_delete["mp_ctf_spawn_allies_start"] = 1;
	classnames_delete["mp_ctf_spawn_axis"] = 1;
	classnames_delete["mp_ctf_spawn_axis_start"] = 1;
	classnames_delete["mp_dm_spawn"] = 1;
	classnames_delete["mp_dom_spawn"] = 1;
	classnames_delete["mp_dom_spawn_allies_start"] = 1;
	classnames_delete["mp_dom_spawn_axis_start"] = 1;
	classnames_delete["mp_global_intermission"] = 0;
	classnames_delete["mp_sab_spawn_allies"] = 1;
	classnames_delete["mp_sab_spawn_allies_start"] = 1;
	classnames_delete["mp_sab_spawn_axis"] = 1;
	classnames_delete["mp_sab_spawn_axis_start"] = 1;
	classnames_delete["mp_sd_spawn_attacker"] = 1;
	classnames_delete["mp_sd_spawn_defender"] = 1;
	classnames_delete["mp_tdm_spawn"] = 1;
	classnames_delete["mp_tdm_spawn_allies_start"] = 1;
	classnames_delete["mp_tdm_spawn_axis_start"] = 1;
	classnames_delete["script_brushmodel"] = 0;
	classnames_delete["script_model"] = 0;
	classnames_delete["script_origin"] = 1;
	classnames_delete["script_struct"] = 1;
	classnames_delete["trigger_hurt"] = 1;
	classnames_delete["trigger_multiple"] = 1;
	classnames_delete["trigger_radius"] = 1;
	classnames_delete["trigger_use_touch"] = 1;
	classnames_delete["worldspawn"] = 0;

	// We need to keep at least one of each tdm spawn
	classnames_keep_one = [];
	classnames_keep_one["mp_tdm_spawn"] = false;
	classnames_keep_one["mp_tdm_spawn_allies_start"] = false;
	classnames_keep_one["mp_tdm_spawn_axis_start"] = false;

	// List of targetnames to delete, 1 to delete, 0 to keep
	targetname_delete = [];
	targetname_delete["ctf_flag_allies"] = 1;
	targetname_delete["ctf_flag_axis"] = 1;
	targetname_delete["exploder"] = 1;
	targetname_delete["flag_descriptor"] = 1;
	targetname_delete["heli_crash_start"] = 1;
	targetname_delete["heli_dest"] = 1;
	targetname_delete["heli_loop_start"] = 1;
	targetname_delete["heli_start"] = 1;
	targetname_delete["minimap_corner"] = 0; // mini map preview
	targetname_delete["sab_bomb"] = 1;
	targetname_delete["sd_bomb"] = 1;

	ents = getentarray();

	for (i = 0; i < ents.size; i++)
	{
		dodelete = false;

		if (isdefined(classnames_delete[ents[i].classname]) && classnames_delete[ents[i].classname] == 1)
		{
			if (isdefined(classnames_keep_one[ents[i].classname]) && classnames_keep_one[ents[i].classname] == false)
			{
				classnames_keep_one[ents[i].classname] = true;
			}
			else
				dodelete = true;
		}

		if (isdefined(targetname_delete[ents[i].targetname]) && targetname_delete[ents[i].targetname] == 1)
			dodelete = true;

		// HQ phone model
		if (ents[i].classname == "script_model" && ents[i].model == "com_cellphone_on")
			dodelete = true;

		if (dodelete)
			ents[i] delete ();
	}
}

get_forge_models()
{
	models = [];
	models["bc_hesco_barrier_med"] = [];
	models["com_bomb_objective"] = [];
	models["com_laptop_2_open"] = [];
	models["com_plasticcase_beige_big"] = [];

	models["pipe"] = [];
	models["terrain"] = [];
	models["arch"] = [];
	models["fuel_tanker"] = [];
	models["fence_piece"] = [];

	script_models = getentarray("script_model", "classname");
	script_brushmodels = getentarray("script_brushmodel", "classname");

	for (i = 0; i < script_models.size; i++)
	{
		if (script_models[i].model == "com_bomb_objective")
		{
			for (j = 0; j < script_brushmodels.size; j++)
			{
				if (!isdefined(script_brushmodels[j].script_gameobjectname))
					continue;

				if (script_brushmodels[j].script_gameobjectname != script_models[i].script_gameobjectname)
					continue;

				if (distance(script_models[i].origin, script_brushmodels[j].origin) > 80)
					continue;

				script_models[i].script_brushmodel = script_brushmodels[j];
				script_brushmodels[j] linkto(script_models[i]);
				models["com_bomb_objective"][models["com_bomb_objective"].size] = script_models[i];
			}
		}

		if (script_models[i].model == "com_laptop_2_open")
		{
			choices = [];
			for (j = 0; j < script_brushmodels.size; j++)
			{
				if (script_brushmodels[j].script_gameobjectname != "hq")
					continue;

				if (!isdefined(script_brushmodels[j].targetname) || script_brushmodels[j].targetname != script_models[i].target)
					continue;

				choices[choices.size] = script_brushmodels[j];
			}

			AssertEx(choices.size == 2, "Expected 2 brush choices for com_laptop_2_open, got " + choices.size);
			// choose the higher Zorigin
			if (choices[0].origin[2] > choices[1].origin[2])
			{
				script_models[i].script_brushmodel = choices[0];
				choices[0] linkto(script_models[i]);
			}
			else
			{
				script_models[i].script_brushmodel = choices[1];
				choices[1] linkto(script_models[i]);
			}

			models["com_laptop_2_open"][models["com_laptop_2_open"].size] = script_models[i];
		}

		if (script_models[i].model == "com_plasticcase_beige_big")
		{
			choices = [];
			for (j = 0; j < script_brushmodels.size; j++)
			{
				if (!isdefined(script_brushmodels[j].script_gameobjectname))
					continue;

				if (script_brushmodels[j].script_gameobjectname != "hq")
					continue;

				if (!isdefined(script_brushmodels[j].targetname) || script_brushmodels[j].targetname != script_models[i].targetname)
					continue;

				choices[choices.size] = script_brushmodels[j];
			}

			AssertEx(choices.size == 2, "Expected 2 brush choices for com_plasticcase_beige_big, got " + choices.size);
			// choose the lower Zorigin
			if (choices[0].origin[2] < choices[1].origin[2])
			{
				script_models[i].script_brushmodel = choices[0];
				choices[0] linkto(script_models[i]);
			}
			else
			{
				script_models[i].script_brushmodel = choices[1];
				choices[1] linkto(script_models[i]);
			}

			models["com_plasticcase_beige_big"][models["com_plasticcase_beige_big"].size] = script_models[i];
		}
	}

	if (getdvar("mapname") == "mp_crossfire")
	{
		// there are 3 bc_hesco_barrier_med script_models linked to 1 script_brushmodel
		for (i = 0; i < script_brushmodels.size; i++)
		{
			if (!isdefined(script_brushmodels[i].script_gameobjectname))
				continue;

			if (script_brushmodels[i].script_gameobjectname != "dom")
				continue;

			bc_hesco_barrier_med_script_brushmodel = script_brushmodels[i];
			bc_hesco_barrier_med_script_models = [];

			for (j = 0; j < script_models.size; j++)
			{
				if (script_models[j].model == "bc_hesco_barrier_med")
				{
					if (distance(script_models[j].origin, bc_hesco_barrier_med_script_brushmodel.origin) < 80)
					{
						script_models[j] linkto(bc_hesco_barrier_med_script_brushmodel);
						script_models[j].forge_parent = bc_hesco_barrier_med_script_brushmodel;
						script_models[j].forge_enabled = true;
						bc_hesco_barrier_med_script_models[bc_hesco_barrier_med_script_models.size] = script_models[j];
					}
				}
			}

			assertex(bc_hesco_barrier_med_script_models.size == 3, "Expected 3 bc_hesco_barrier_med script_models linked to 1 script_brushmodel, got " + bc_hesco_barrier_med_script_models.size);
			bc_hesco_barrier_med_script_brushmodel.forge_children = bc_hesco_barrier_med_script_models;

			models["bc_hesco_barrier_med"][models["bc_hesco_barrier_med"].size] = bc_hesco_barrier_med_script_brushmodel;
		}
	}

	if (getdvar("mapname") == "mp_bog")
	{
		models["arch"][models["arch"].size] = getentbyorigin((3461, -149, 176));
	}

	if (getdvar("mapname") == "mp_cargoship")
	{
		models["fuel_tanker"][models["fuel_tanker"].size] = getentbyorigin((1300, 61, 104));
	}

	if (getdvar("mapname") == "mp_countdown")
	{
		models["fence_piece"][models["fence_piece"].size] = getentbyorigin((-573, 2956, 32));
		models["fence_piece"][models["fence_piece"].size] = getentbyorigin((-574, 2958, 35));
		models["fence_piece"][models["fence_piece"].size] = getentbyorigin((-581, 2961, -18));
		models["fence_piece"][models["fence_piece"].size] = getentbyorigin((-568, 2953, -18));
		models["fence_piece"][models["fence_piece"].size] = getentbyorigin((-505, 2918, -37));
		models["fence_piece"][models["fence_piece"].size] = getentbyorigin((-506, 2918, 82));
		models["fence_piece"][models["fence_piece"].size] = getentbyorigin((-474, 2900, 36));
		models["fence_piece"][models["fence_piece"].size] = getentbyorigin((-439, 2880, 82));
		models["fence_piece"][models["fence_piece"].size] = getentbyorigin((-505, 2918, -11));
		models["fence_piece"][models["fence_piece"].size] = getentbyorigin((-439, 2880, -12));
	}

	if (getdvar("mapname") == "mp_farm")
	{
		models["pipe"] = getEntArray("gas_station", "targetname");
	}

	if (getdvar("mapname") == "mp_showdown")
	{
		models["terrain"][models["terrain"].size] = getentbyorigin((-1040, 74, 82), 1);
		models["terrain"][models["terrain"].size] = getentbyorigin((-1040, 74, 82), 2);
		models["terrain"][models["terrain"].size] = getentbyorigin((-778, 684, 82), 1);
		models["terrain"][models["terrain"].size] = getentbyorigin((-778, 684, 82), 2);
	}

	// capture the starting positions
	for (i = 0; i < models.size; i++)
	{
		modelName = getarraykeys(models)[i];
		for (j = 0; j < models[modelName].size; j++)
		{
			modelEnt = models[modelName][j];
			modelEnt.startOrigin = modelEnt.origin;
			modelEnt.startAngles = modelEnt.angles;
			modelEnt.forge_enabled = true;
		}
	}

	// Remove empty model types
	to_return = [];
	modelnames = getarraykeys(models);
	for (i = 0; i < modelnames.size; i++)
	{
		if (models[modelnames[i]].size == 0)
			continue;
		else
			to_return[modelnames[i]] = models[modelnames[i]];
	}

	return to_return;
}

/**
 * Finds the nth entity with the specified origin.
 *
 * @param origin - The target origin to match.
 * @param matchNumber - (Optional) The 1-based position of the match to return. Defaults to 1.
 * @returns The entity at the specified matchNumber, or undefined if not found.
 */
getentbyorigin(origin, matchNumber)
{
	if (!isdefined(matchNumber))
		matchNumber = 1;

	matchCount = 0;

	ents = getentarray();
	for (i = 0; i < ents.size; i++)
	{
		if (ents[i].origin == origin)
		{
			matchCount++;
			if (matchCount == matchNumber)
				return ents[i];
		}
	}
}

RestoreBrushCollision()
{
	SetDvar("noclip_brushes", "");
	IPrintLn("^2Collision restored for all brushes.");
}

RemoveAllBrushCollision()
{
	SetDvar("noclip_brushes", "*");
	IPrintLn("^2Collision disabled for all brushes.");
}

RemoveBrushCollisionForPoint(point)
{
	brushIndices = getplayerclipbrushescontainingpoint(point);
	if(brushIndices.size == 0)
	{
		self IPrintLn("^1No playerclip brushes found at this point.");
		return;
	}

	brush_str = "";

	for( i = 0; i < brushIndices.size; i++)
	{
		brushIndex = brushIndices[i];
		if(brushIndex == 0)
			brush_str += brushIndex;
		else
			brush_str += " " + brushIndex;
	}

	currentValue = GetDvar("noclip_brushes");
	newValue = currentValue + " " + brush_str;
	SetDvar("noclip_brushes", newValue);
	IPrintLn("^2Disabled collision on brushes: " + newValue);
}

disablecollisionforbrushcontainingorigin_wrapper()
{
	// Get all playerclip brushes that contain the player's position
	point = self.origin;
	point = point - (0, 0, 1); // Adjust the point slightly above the player to avoid ground collision
	RemoveBrushCollisionForPoint(point);
}

/**
 * Check if a button is pressed.
 */
button_pressed(button)
{
	switch (ToLower(button))
	{
	case "ads":
		return self adsbuttonpressed();
	case "attack":
		return self attackbuttonpressed();
	case "frag":
		return self fragbuttonpressed();
	case "holdbreath":
		return self holdbreathbuttonpressed();
	case "melee":
		return self meleebuttonpressed();
	case "nightvision":
		return isdefined(self.nightVisionButtonPressedTime) && (getTime() - self.nightVisionButtonPressedTime < 200);
	case "smoke":
		return self secondaryoffhandbuttonpressed();
	case "use":
		return self usebuttonpressed();
	default:
		self iprintln("^1Unknown button " + button);
		return false;
	}
}

/**
 * Check if a button is pressed twice within 500ms.
 */
button_pressed_twice(button)
{
	if (self button_pressed(button))
	{
		// Wait for the button to be released after the first press
		while (self button_pressed(button))
		{
			wait 0.05;
		}

		// Now, wait for a second press within 500ms
		for (elapsed_time = 0; elapsed_time < 0.5; elapsed_time += 0.05)
		{
			if (self button_pressed(button))
			{
				// Ensure it was released before this second press
				return true;
			}

			wait 0.05;
		}
	}
	return false;
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

get_dvars()
{
	dvars = [];

	// Alphabetically sorted by key
	dvars["bg_bobMax"] = spawnstruct();
	dvars["bg_bobMax"].type = "slider";
	dvars["bg_bobMax"].name = "bg_bobMax";
	dvars["bg_bobMax"].default_value = 8;
	dvars["bg_bobMax"].min = 0;
	dvars["bg_bobMax"].max = 36;
	dvars["bg_bobMax"].step = 1;

	dvars["cg_drawGun"] = spawnstruct();
	dvars["cg_drawGun"].type = "boolean";
	dvars["cg_drawGun"].name = "cg_drawGun";
	dvars["cg_drawGun"].default_value = 1;

	dvars["cg_drawSpectatorMessages"] = spawnstruct();
	dvars["cg_drawSpectatorMessages"].type = "boolean";
	dvars["cg_drawSpectatorMessages"].name = "cg_drawSpectatorMessages";
	dvars["cg_drawSpectatorMessages"].default_value = 1;

	dvars["cg_fov"] = spawnstruct();
	dvars["cg_fov"].type = "slider";
	dvars["cg_fov"].name = "cg_fov";
	dvars["cg_fov"].default_value = 65;
	dvars["cg_fov"].min = 65;
	dvars["cg_fov"].max = 90;
	dvars["cg_fov"].step = 1;

	dvars["cg_fovScale"] = spawnstruct();
	dvars["cg_fovScale"].type = "slider";
	dvars["cg_fovScale"].name = "cg_fovScale";
	dvars["cg_fovScale"].default_value = 1;
	dvars["cg_fovScale"].min = 0.2;
	dvars["cg_fovScale"].max = 2;
	dvars["cg_fovScale"].step = 0.1;

	dvars["cg_thirdPerson"] = spawnstruct();
	dvars["cg_thirdPerson"].type = "boolean";
	dvars["cg_thirdPerson"].name = "cg_thirdPerson";
	dvars["cg_thirdPerson"].default_value = 0;

	dvars["cg_thirdPersonAngle"] = spawnstruct();
	dvars["cg_thirdPersonAngle"].type = "slider";
	dvars["cg_thirdPersonAngle"].name = "cg_thirdPersonAngle";
	dvars["cg_thirdPersonAngle"].default_value = 356;
	dvars["cg_thirdPersonAngle"].min = -180;
	dvars["cg_thirdPersonAngle"].max = 360;
	dvars["cg_thirdPersonAngle"].step = 1;

	dvars["cg_thirdPersonRange"] = spawnstruct();
	dvars["cg_thirdPersonRange"].type = "slider";
	dvars["cg_thirdPersonRange"].name = "cg_thirdPersonRange";
	dvars["cg_thirdPersonRange"].default_value = 120;
	dvars["cg_thirdPersonRange"].min = 0;
	dvars["cg_thirdPersonRange"].max = 1024;
	dvars["cg_thirdPersonRange"].step = 1;

	dvars["jump_slowdownEnable"] = spawnstruct();
	dvars["jump_slowdownEnable"].scope = "global";
	dvars["jump_slowdownEnable"].type = "boolean";
	dvars["jump_slowdownEnable"].name = "jump_slowdownEnable";
	dvars["jump_slowdownEnable"].default_value = 1;

	dvars["r_blur"] = spawnstruct();
	dvars["r_blur"].type = "slider";
	dvars["r_blur"].name = "r_blur";
	dvars["r_blur"].default_value = 0;
	dvars["r_blur"].min = 0;
	dvars["r_blur"].max = 32;
	dvars["r_blur"].step = 0.2;

	dvars["r_dof_enable"] = spawnstruct();
	dvars["r_dof_enable"].type = "boolean";
	dvars["r_dof_enable"].name = "r_dof_enable";
	dvars["r_dof_enable"].default_value = 1;

	dvars["r_fog"] = spawnstruct();
	dvars["r_fog"].type = "boolean";
	dvars["r_fog"].name = "r_fog";
	dvars["r_fog"].default_value = 1;

	dvars["r_fullbright"] = spawnstruct();
	dvars["r_fullbright"].type = "boolean";
	dvars["r_fullbright"].name = "r_fullbright";
	dvars["r_fullbright"].default_value = 0;

	dvars["r_zfar"] = spawnstruct();
	dvars["r_zfar"].type = "slider";
	dvars["r_zfar"].name = "r_zfar";
	dvars["r_zfar"].default_value = 0;
	dvars["r_zfar"].min = 0;
	dvars["r_zfar"].max = 4000;
	dvars["r_zfar"].step = 500;

	dvars["g_compassShowEnemies"] = spawnstruct();
	dvars["g_compassShowEnemies"].type = "boolean";
	dvars["g_compassShowEnemies"].name = "g_compassShowEnemies";
	dvars["g_compassShowEnemies"].default_value = 0;

	return dvars;
}

get_maps()
{
	maps = [];
	// Alphabetically sorted by value
	maps["mp_convoy"] = "Ambush";
	maps["mp_backlot"] = "Backlot";
	maps["mp_bloc"] = "Bloc";
	maps["mp_bog"] = "Bog";
	maps["mp_broadcast"] = "Broadcast";
	maps["mp_carentan"] = "Chinatown";
	maps["mp_countdown"] = "Countdown";
	maps["mp_crash"] = "Crash";
	maps["mp_creek"] = "Creek";
	maps["mp_crossfire"] = "Crossfire";
	maps["mp_citystreets"] = "District";
	maps["mp_farm"] = "Downpour";
	maps["mp_killhouse"] = "Killhouse";
	maps["mp_overgrown"] = "Overgrown";
	maps["mp_pipeline"] = "Pipeline";
	maps["mp_shipment"] = "Shipment";
	maps["mp_showdown"] = "Showdown";
	maps["mp_strike"] = "Strike";
	maps["mp_vacant"] = "Vacant";
	maps["mp_cargoship"] = "Wet Work";
	if (level.xenon)
		maps["mp_crash_snow"] = "Winter Crash";

	return maps;
}

get_player_models()
{
	mapname = getDvar("mapname");

	models = [];
	models["com_bomb_objective"] = "Bomb";
	models["com_bomb_objective_d"] = "Bomb destroyed";
	models["com_cellphone_on"] = "Cellphone";
	models["com_laptop_2_open"] = "Laptop";
	models["com_plasticcase_beige_big"] = "Crate";

	models["vehicle_80s_sedan1_brn_destructible_mp"] = "Car brown";
	models["vehicle_80s_sedan1_silv_destructible_mp"] = "Car silver";
	models["vehicle_80s_wagon1_yel_destructible_mp"] = "Car yellow";

	// TODO: add more map specific models
	if (mapname == "mp_convoy")
	{
		models["foliage_tree_palm_bushy_1"] = "Palm tree 1";
		models["foliage_tree_palm_bushy_3"] = "Palm tree 2";
	}

	if (mapname == "mp_crossfire")
	{
		models["bc_hesco_barrier_med"] = "Hesco barrier";
	}

	return models;
}

/**
 * Changes the player model.
 */
change_player_model(model)
{
	self detach_head();	   // Detach the head from the player model
	self setViewModel(""); // Remove viewhands
	// Doesn't stay detached after changing weapons
	maps\mp\gametypes\_weapons::detach_all_weapons(); // Detach all stowed weapons
	self setModel(model);
}

/**
 * Detaches the head from the player model.
 */
detach_head()
{
	count = self getattachsize();
	for (index = 0; index < count; index++)
	{
		head = self getattachmodelname(index);

		if (starts_with(head, "head"))
		{
			self detach(head);
			break;
		}
	}
}

/**
 * Returns true if the string starts with the prefix.
 */
starts_with(string, prefix)
{
	if (string == prefix)
		return true;
	if (prefix.size > string.size)
		return false;

	for (index = 0; index < prefix.size; index++)
	{
		if (string[index] != prefix[index])
			return false;
	}

	return true;
}

get_saved_client_dvar(dvar, default_value)
{
	value = self.cj["dvars"][dvar];
	if (!isdefined(value))
		return default_value;
	else
		return value;
}

set_saved_client_dvar(dvar, value)
{
	self.cj["dvars"][dvar] = value;
	self setClientDvar(dvar, value);

	default_value = undefined;
	if (isdefined(level.DVARS[dvar]))
		default_value = level.DVARS[dvar].default_value;

	msg = dvar + ": " + value;
	if (value == default_value)
		msg += " [DEFAULT]";
	self iprintln(msg);
}

isDvarStructValid(dvar)
{
	// all must have a name, type
	if (!isdefined(dvar) || !isdefined(dvar.type) || !isdefined(dvar.name))
		return false;

	// type specific checks
	if (dvar.type == "slider")
	{
		if (!isdefined(dvar.default_value) || !isdefined(dvar.min) || !isdefined(dvar.max) || !isdefined(dvar.step))
			return false;
	}
	else if (dvar.type == "boolean")
	{
		if (!isdefined(dvar.default_value))
			return false;
	}
	return true;
}

// Function to calculate and update the cursor position based on dvar value
updateCursorPosition(dvar, dvarValue, sliderCursor, centerXPosition, railWidth, cursorWidth)
{
	// Calculate normalized position (0 to 1) on the rail
	normalizedPosition = (dvarValue - dvar.min) / (dvar.max - dvar.min);
	// Calculate actual x position on the rail
	sliderCursor.x = centerXPosition + int(normalizedPosition * (railWidth - cursorWidth));
}

// TODO: more options
// - reset to default
// - add a label to the slider?
// - ignore main menu button presses when the slider controls are open
slider_start(dvar)
{
	self endon("disconnect");
	self endon("end_respawn");

	// self menuAction("CLOSE");

	if (!isDvarStructValid(dvar) || dvar.type != "slider")
	{
		self iprintln("^1dvar struct is invalid");
		return;
	}

	if (!isdefined(self.cj["slider_hud"]))
		self.cj["slider_hud"] = [];
	else
		self slider_hud_destroy();

	// call this on a fresh game to get the default value
	// self iprintln("DEFAULT GAME VALUE " + dvar.name + " " + getdvar(dvar.name));

	// -- Background
	backgroundWidth = level.SCREEN_MAX_WIDTH;
	backgroundHeight = 50;
	centerYPosition = (level.SCREEN_MAX_HEIGHT - backgroundHeight) / 2;

	self.cj["slider_hud"]["background"] = newClientHudElem(self);
	self.cj["slider_hud"]["background"].elemType = "icon";
	self.cj["slider_hud"]["background"].color = (0, 0, 0);
	self.cj["slider_hud"]["background"].alpha = 0.5;
	self.cj["slider_hud"]["background"] setShader("white", backgroundWidth, backgroundHeight);
	self.cj["slider_hud"]["background"].x = 0;
	self.cj["slider_hud"]["background"].y = centerYPosition;
	self.cj["slider_hud"]["background"].alignX = "left";
	self.cj["slider_hud"]["background"].alignY = "top";
	self.cj["slider_hud"]["background"].horzAlign = "fullscreen";
	self.cj["slider_hud"]["background"].vertAlign = "fullscreen";

	// -- Rail
	railWidth = int(level.SCREEN_MAX_WIDTH * 0.75);
	railHeight = 4;
	centerXPosition = (level.SCREEN_MAX_WIDTH - railWidth) / 2;
	centerYPosition = (level.SCREEN_MAX_HEIGHT - railHeight) / 2;

	self.cj["slider_hud"]["rail"] = newClientHudElem(self);
	self.cj["slider_hud"]["rail"].elemType = "icon";
	self.cj["slider_hud"]["rail"].alpha = 0.75;
	self.cj["slider_hud"]["rail"] setShader("white", railWidth, railHeight);
	self.cj["slider_hud"]["rail"].x = centerXPosition;
	self.cj["slider_hud"]["rail"].y = centerYPosition;
	self.cj["slider_hud"]["rail"].alignX = "left";
	self.cj["slider_hud"]["rail"].alignY = "top";
	self.cj["slider_hud"]["rail"].horzAlign = "fullscreen";
	self.cj["slider_hud"]["rail"].vertAlign = "fullscreen";

	// -- Cursor
	cursorWidth = 3;
	cursorHeight = int(backgroundHeight / 2);
	// Start position aligned with the beginning of the rail
	cursorStartXPosition = centerXPosition; // This aligns it to the start of the rail
	// Centered vertically with respect to the rail
	cursorYPosition = centerYPosition - (cursorHeight - railHeight) / 2;

	self.cj["slider_hud"]["cursor"] = newClientHudElem(self);
	self.cj["slider_hud"]["cursor"].elemType = "icon";
	self.cj["slider_hud"]["cursor"].color = self.themeColor; // Use the theme color
	self.cj["slider_hud"]["cursor"].alpha = 0;				 // Hide the cursor initially
	self.cj["slider_hud"]["cursor"] setShader("white", cursorWidth, cursorHeight);
	self.cj["slider_hud"]["cursor"].x = cursorStartXPosition;
	self.cj["slider_hud"]["cursor"].y = cursorYPosition;
	self.cj["slider_hud"]["cursor"].alignX = "left";
	self.cj["slider_hud"]["cursor"].alignY = "top";
	self.cj["slider_hud"]["cursor"].horzAlign = "fullscreen";
	self.cj["slider_hud"]["cursor"].vertAlign = "fullscreen";

	dvarValue = self get_saved_client_dvar(dvar.name, dvar.default_value);

	// Initialize cursor position based on the default dvar value
	updateCursorPosition(dvar, dvarValue, self.cj["slider_hud"]["cursor"], centerXPosition, railWidth, cursorWidth);

	self.cj["slider_hud"]["cursor"].alpha = 1; // Show the cursor after it has been positioned

	self.cj["slider_hud"]["value"] = createFontString("default", 3);
	self.cj["slider_hud"]["value"] setPoint("CENTER", "CENTER", 0, -50);
	self.cj["slider_hud"]["value"] SetValue(dvarValue);

	instructions = [];
	instructions[instructions.size] = "[{+smoke}] Decrease";
	instructions[instructions.size] = "[{+frag}] Increase";
	instructions[instructions.size] = "[{+melee}] Save and exit";

	instructionsString = "";
	for (i = 0; i < instructions.size; i++)
		instructionsString += instructions[i] + "\n";

	self.cj["slider_hud"]["instructions"] = createFontString("default", 1.4);
	self.cj["slider_hud"]["instructions"] setPoint("TOPLEFT", "TOPLEFT", -30, -20);
	self.cj["slider_hud"]["instructions"] setText(instructionsString);

	for (;;)
	{
		if (self fragbuttonpressed() || self secondaryoffhandbuttonpressed())
		{
			if (self fragbuttonpressed())
			{
				dvarValue += dvar.step;
				if (dvarValue > dvar.max)
					dvarValue = dvar.min; // Wrap around to min
			}
			else if (self secondaryoffhandbuttonpressed())
			{
				dvarValue -= dvar.step;
				if (dvarValue < dvar.min)
					dvarValue = dvar.max; // Wrap around to max
			}

			updateCursorPosition(dvar, dvarValue, self.cj["slider_hud"]["cursor"], centerXPosition, railWidth, cursorWidth);
			self.cj["slider_hud"]["value"] SetValue(dvarValue);
			self set_saved_client_dvar(dvar.name, dvarValue);

			wait 0.05; // Prevent rapid firing
		}
		else if (self meleebuttonpressed())
		{
			self set_saved_client_dvar(dvar.name, dvarValue);
			self slider_hud_destroy();

			// self menuAction("OPEN");
			return;
		}

		wait 0.05;
	}
}

slider_hud_destroy()
{
	if (!isdefined(self.cj["slider_hud"]))
		return;
	keys = getarraykeys(self.cj["slider_hud"]);
	for (i = 0; i < keys.size; i++)
	{
		if (isdefined(self.cj["slider_hud"][keys[i]]))
			self.cj["slider_hud"][keys[i]] destroy();
	}
}

toggle_boolean_dvar(dvar)
{
	if (!isDvarStructValid(dvar) || dvar.type != "boolean")
	{
		self iprintln("^1dvar struct is invalid");
		return;
	}

	dvarValue = self get_saved_client_dvar(dvar.name, dvar.default_value);

	if (dvarValue == 0)
		self set_saved_client_dvar(dvar.name, 1);
	else
		self set_saved_client_dvar(dvar.name, 0);
}

reset_all_client_dvars()
{
	dvars = getarraykeys(level.DVARS);
	for (i = 0; i < dvars.size; i++)
	{
		dvar = level.DVARS[dvars[i]];
		self set_saved_client_dvar(dvar.name, dvar.default_value);
	}
}

/**
 * Sets up the loadout for the player.
 */
cj_setup_loadout(printInfo)
{
	if (!isdefined(printInfo))
		printInfo = true;

	self clearPerks();
	self takeAllWeapons();

	// wait 0.05;

	self giveWeapon(self.cj["loadout"].primary, self.cj["loadout"].primaryCamoIndex);
	self giveWeapon(self.cj["loadout"].sidearm);

	self giveWeapon("rpg_mp");

	if (self.cj["loadout"].fastReload)
		self setPerk("specialty_fastreload");

	self SetActionSlot(1, "nightvision");
	self SetActionSlot(3, "weapon", "rpg_mp");

	wait 0.05;

	// Switch to the appropriate weapon
	if (isdefined(self.cj["loadout"].incomingWeapon) && weaponClass(self.cj["loadout"].incomingWeapon) != "pistol")
		self switchtoweapon(self.cj["loadout"].primary);
	else
		self switchtoweapon(self.cj["loadout"].sidearm);

	self.cj["loadout"].incomingWeapon = undefined;

	baseWeapon = strTok(self.cj["loadout"].primary, "_")[0];
	self maps\mp\gametypes\_teams::playerModelForWeapon(baseWeapon);

	// Adjust move speed based on primary weapon type
	moveSpeedScalePercentage = 100;
	// Taken from maps\mp\gametypes\_class::giveLoadout
	switch (weaponClass(self.cj["loadout"].primary))
	{
	case "rifle":
		self setMoveSpeedScale(0.95);
		moveSpeedScalePercentage = 95;
		break;
	case "pistol":
		self setMoveSpeedScale(1.0);
		break;
	case "mg":
		self setMoveSpeedScale(0.875);
		moveSpeedScalePercentage = 87.5;
		break;
	case "smg":
		self setMoveSpeedScale(1.0);
		break;
	case "spread":
		self setMoveSpeedScale(1.0);
		break;
	default:
		self setMoveSpeedScale(1.0);
		break;
	}

	if (printInfo)
		self iprintln("Loadout move speed: " + moveSpeedScalePercentage + " percent");
}

toggle_fast_reload()
{
	self.cj["loadout"].fastReload = !self.cj["loadout"].fastReload;
	if (self.cj["loadout"].fastReload)
		self iprintln("Fast reload [^2ON^7]");
	else
		self iprintln("Fast reload [^1OFF^7]");

	self cj_setup_loadout(false);
}

give_camo(index)
{
	self.cj["loadout"].primaryCamoIndex = index;
	self.cj["loadout"].incomingWeapon = self.cj["loadout"].primary;
	self cj_setup_loadout(false);
}

replace_weapon(weapon)
{
	if (weaponClass(weapon) != "pistol")
	{
		self.cj["loadout"].primary = weapon;
		self.cj["loadout"].primaryCamoIndex = 0;
	}
	else
		self.cj["loadout"].sidearm = weapon;

	self.cj["loadout"].incomingWeapon = weapon;
	self cj_setup_loadout();
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
		if (self == level.players[0])
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

		self addMenuOption("host_menu", "Toggle Multi Bouncing", ::toggleMultiBouncing);
		self addMenuOption("host_menu", "Toggle Auto Bhop", ::toggleBhopAuto);
		self addMenuOption("host_menu", "Toggle PC lookdown pitch", ::toggle_pc_lookdown_pitch);

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
	self addMenuOption("player_settings", "Distance HUD", ::toggle_hud_display, "distance");
	self addMenuOption("player_settings", "Speed HUD", ::toggle_hud_display, "speed");
	self addMenuOption("player_settings", "Height HUD", ::toggle_hud_display, "z_origin");

	self addMenuOption("player_settings", "Jump Crouch", ::toggleJumpCrouch);
	self addMenuOption("player_settings", "Lean Toggle", ::LeanBindToggle);
	self addMenuOption("player_settings", "Cycle Visions", ::CycleVision);
	self addMenuOption("player_settings", "Revert Vision", ::RevertVision);


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
		self addMenuOption("barrier_menu", "Remove All Barriers", ::RemoveAllBrushCollision);
		self addMenuOption("barrier_menu", "Disable Collision at origin", ::disablecollisionforbrushcontainingorigin_wrapper);
		self addMenuOption("barrier_menu", "Restore Barriers", ::RestoreBrushCollision);

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
				self savePos();
				wait .2;
			}
			else if (!self.ufo && self button_pressed("smoke"))
			{
				savenum = self.cj["savenum"];
				if(savenum == 0)
				{
					self IPrintLn("No positions saved yet.");
				}
				else
				{
					// Load last position immediately
					self loadPos(savenum);
					// If user holds the button, load previous positions when pressing use
					while (self button_pressed("smoke"))
					{
						if(self button_pressed("use"))
						{
							savenum = savenum - 1;
							if (savenum < 1)
								savenum = self.cj["savenum"];
							self loadPos(savenum);
							self IPrintLn("Position " + savenum + " loaded");
						}
						wait .1;
					}
				}

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

savePos()
{
	if (!self isOnGround())
		return;

	self.cj["savenum"] += 1;

	self.cj["settings"]["rpg_switched"] = false;
	self.cj["saves"]["org"][self.cj["savenum"]] = self.origin;
	self.cj["saves"]["ang"][self.cj["savenum"]] = self getPlayerAngles();

	self IPrintLn("Position " + self.cj["savenum"] + " saved");

	self.kills += 1;
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

	self.assists += 1;
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
	// test clients stay in connecting state when changing map
	// so we need to kick them
	kickAllBots();
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

toggleMultiBouncing()
{
	value = getDvarInt("pm_multi_bounce");
	if (value == 0)
	{
		setDvar("pm_multi_bounce", 1);
		iprintln("Multi Bouncing [^2ON^7]");
	}
	else
	{
		setDvar("pm_multi_bounce", 0);
		iprintln("Multi Bouncing [^1OFF^7]");
	}
}

toggleBhopAuto()
{
	value = getDvarInt("pm_bhop_auto");
	if (value == 0)
	{
		setDvar("pm_bhop_auto", 1);
		iprintln("Auto Bhop [^2ON^7]");
	}
	else
	{
		setDvar("pm_bhop_auto", 0);
		iprintln("Auto Bhop [^1OFF^7]");
	}
}
