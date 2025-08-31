#include maps\mp\gametypes\_hud_util;
#include maps\mp\_utility;

init()
{
	precacheShader("menu_background_mp_tank");
	precacheShader("minimap_background");
	setDvar("scr_tdm_timelimit", 0);
	setDvar("scr_tdm_scorelimit", 0);

	setDvar("player_sprintUnlimited", 1);
	setDvar("jump_slowdownEnable", 0);

	setDvar("bg_fallDamageMaxHeight", 9999);
	setDvar("bg_fallDamageMinHeight", 9998);

	setDvar("scr_showperksonspawn", 0); // Remove perks icons shown on spawn
	setDvar("scr_game_hardpoints", 0);	// Remove killstreaks

	// Prevent bots from moving
	setDvar("sv_botsRandomInput", 0);
	setDvar("sv_botsPressAttackBtn", 0);

	level thread delete_death_barriers();

	level thread onPlayerConnect();

	thread maps\mp\gametypes\menu::init();

}

onPlayerConnect()
{
	for (;;)
	{
		level waittill("connecting", player);

		// Don't setup bots
		if (isDefined(player.pers["isBot"]))
			continue;

		player init_client_dvars();
		player init_player_once();

		player thread onPlayerSpawned();
	}
}

onPlayerSpawned()
{
	self endon("disconnect");

	for (;;)
	{
		self waittill("spawned_player");

		self thread watch_player_buttons();
		self thread replenish_ammo();
		self.god = true; // Enable god mode

		// Remove all grenades
		self TakeWeapon("frag_grenade_mp");
		self TakeWeapon("m8_white_smoke_mp");
		self TakeWeapon("sticky_grenade_mp");
		self TakeWeapon("tabun_gas_mp");
		self TakeWeapon("molotov_mp");

		self setClientDvar("cg_drawCrosshair", 0);
		self setClientDvar("ui_hud_hardcore", 1);			// Hardcore HUD
		self setClientDvar("g_compassShowEnemies", 0);		// Hide compass from HUD
		self setClientDvar("player_bayonetLaunchProof", 0); // Allow bayonet launches and bounces
		self setClientDvar("bg_bobIdle", 0);				// Disable idle gun sway

		self toggle_meter_hud();

		// whips edits
		self thread EnableAllChanges();
	}
}

init_player_once()
{
	if (isdefined(self.init_player))
		return;

	self.cj = [];
	self.cj["bots"] = [];
	self.cj["botnumber"] = 0;
	self.cj["maxbots"] = 4;
	self.cj["menu_open"] = false;

	self.init_player = true;
}

init_client_dvars()
{
	self setClientDvar("loc_warnings", 0); // Disable unlocalized text warnings

	self setClientDvar("cg_drawSpectatorMessages", 0);
	self setClientDvar("cg_descriptiveText", 0);		  // Remove spectator button icons and text
	self setClientDvar("player_spectateSpeedScale", 1.5); // Faster movement in spectator

	self setclientdvar("player_view_pitch_up", 89.9); // Allow looking straight up
	self setClientDvars("fx_enable", 0);			  // Disable FX (RPG smoke etc)

	self setClientDvar("aim_automelee_range", 0); // Remove melee lunge on enemy players
	// Remove aim assist on enemy players
	self setClientDvar("aim_slowdown_enabled", 0);
	self setClientDvar("aim_lockon_enabled", 0);

	// Remove overhead names and ranks on enemy players
	self setClientDvar("cg_overheadNamesSize", 0);
	self setClientDvar("cg_overheadRankSize", 0);

	self setClientDvar("cg_drawCrosshairNames", 0);
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
	case "melee":
		return self meleebuttonpressed();
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

ToggleUFO()
{
	if (self.ufo)
	{
		self.ufo = false;
		self IPrintLn("ufo mode OFF");
	}
	else
	{
		self.ufo = true;
		self IPrintLn("ufo mode ON");
	}
}

watch_player_buttons()
{
	self endon("disconnect");
	self endon("death");
	for (;;)
	{
		if (!(self maps\mp\gametypes\menu::isMenuOpen()))
		{
			if (self GetStance() == "crouch" && self adsButtonPressed() && self meleebuttonpressed())
			{
				self maps\mp\gametypes\menu::menuAction("OPEN");
				wait 0.2;
			}
			else if (self button_pressed("frag"))
			{
				self thread ToggleUFO();
				wait 0.2;
			}
			else if (self button_pressed_twice("melee"))
			{
				self thread save_position(0);
				wait 0.2;
			}
			else if (self button_pressed("smoke"))
			{
				self thread load_position(0);
				wait 0.2;
			}
			// Host only
			else if (self GetEntityNumber() == 0 && self.ufo == true && self button_pressed("use"))
			{
				// Get all playerclip brushes that contain the player's position
				point = self.origin;
				point = point - (0, 0, 1); // Adjust the point slightly above the player to avoid ground collision
				RemoveBrushCollisionForPoint(point);
				wait 0.2;
			}
			// if (self button_pressed_twice("use"))
			// {
			// 	self maps\mp\gametypes\menu::menuAction("OPEN");
			// 	wait 0.2;
			// }
		}
		else
		{
			if (self button_pressed("use"))
			{
				self maps\mp\gametypes\menu::menuAction("SELECT");
				wait.2;
			}
			else if (self button_pressed("melee"))
			{
				self maps\mp\gametypes\menu::menuAction("BACK");
				wait.2;
			}
			else if (self button_pressed("melee"))
			{
				self maps\mp\gametypes\menu::menuAction("CLOSE");
				wait.2;
			}
			else if (self button_pressed("ads"))
			{
				self maps\mp\gametypes\menu::menuAction("UP");
				wait.2;
			}
			else if (self button_pressed("attack"))
			{
				self maps\mp\gametypes\menu::menuAction("DOWN");
				wait.2;
			}
		}

		wait 0.05;
	}
}

/**
 * Constantly replace the players ammo.
 */
replenish_ammo()
{
	self endon("disconnect");
	self endon("death");

	for (;;)
	{
		currentWeapon = self getCurrentWeapon(); // undefined if the player is mantling or on a ladder
		if (isdefined(currentWeapon))
			self giveMaxAmmo(currentWeapon);
		wait 1;
	}
}

toggle_meter_hud()
{
	if (!isdefined(self.cj["meter_hud"]))
		self.cj["meter_hud"] = [];

	// not defined means OFF
	if (!isdefined(self.cj["meter_hud"]["speed"]))
	{
		self.cj["meter_hud"] = [];
		self thread start_hud_speed();
		self thread start_hud_z_origin();
	}
	else
	{
		self notify("end_hud_speed");
		self notify("end_hud_z_origin");

		self.cj["meter_hud"]["speed"] destroy();
		self.cj["meter_hud"]["z_origin"] destroy();
	}
}

start_hud_speed()
{
	self endon("disconnect");
	self endon("end_hud_speed");

	fontScale = 1.4;
	x = 62;
	y = 22;
	alpha = 0.4;

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

start_hud_z_origin()
{
	self endon("disconnect");
	self endon("end_hud_z_origin");

	fontScale = 1.4;
	x = 62;
	y = 36;
	alpha = 0.4;

	self.cj["meter_hud"]["z_origin"] = createFontString("small", fontScale);
	self.cj["meter_hud"]["z_origin"] setPoint("BOTTOMRIGHT", "BOTTOMRIGHT", x, y);
	self.cj["meter_hud"]["z_origin"].alpha = alpha;
	self.cj["meter_hud"]["z_origin"].label = &"z:&&1";

	for (;;)
	{
		self.cj["meter_hud"]["z_origin"] setValue(self.origin[2]);

		wait 0.05;
	}
}

save_position(i)
{
	if (!self isonground())
		return;

	save = spawnstruct();
	save.origin = self.origin;
	save.angles = self getplayerangles();

	self.cj["saves"][i] = save;
}

load_position(i)
{
	self freezecontrols(true);
	wait 0.05;

	save = self.cj["saves"][i];

	self setplayerangles(save.angles);
	self setorigin(save.origin);
	if (self.SatchelRespawn == true)
	{
	}
	wait 0.05;
	self freezecontrols(false);
}

delete_death_barriers()
{
	// Wait for the game to start otherwise the entities won't be spawned
	level waittill("connected");

	ents = getEntArray("trigger_multiple", "classname");
	for (i = 0; i < ents.size; i++)
	{
		ents[i] delete ();
	}

	ents = getEntArray("trigger_radius", "classname");
	for (i = 0; i < ents.size; i++)
	{
		ents[i] delete ();
	}

	ents = getEntArray("trigger_hurt", "classname");
	for (i = 0; i < ents.size; i++)
	{
		ents[i] delete ();
	}
}

EnableAllChanges()
{
	self thread SatchelDpadRight();
	self thread DpadButtonMonitorDown();
	self thread DpadButtonMonitorUp();
}

SatchelDpadRight()
{
	self giveWeapon("satchel_charge_mp");
	self giveMaxAmmo("satchel_charge_mp");
	self setActionSlot(4, "weapon", "satchel_charge_mp");
}

DpadButtonMonitorUp()
{
	self endon("death");
	self endon("disconnect");
	self endon("game_ended");

	self SetActionSlot(1, "squadcommand_mp");

	for (;;)
	{
		self waittill("weapon_change");

		if (self getCurrentWeapon() != "squadcommand_mp")
			continue;

		wait.05;

		self notify("cancel_location");

		self thread spawnSelectedBot();
	}
}

initBot()
{
	bot = addtestclient();
	bot.pers["isBot"] = true;
	bot.pers["name"] = "Harry";
	while (!isDefined(bot.pers["team"]))
		wait 0.05;
	bot notify("menuresponse", game["menu_team"], "axis");
	wait 0.5;
	bot.weaponPrefix = "ak47_mp";
	bot notify("menuresponse", "changeclass", "specops_mp");
	bot waittill("spawned_player");
	bot.selectedClass = true;
	while (bot.sessionstate != "playing")
		wait 0.05;
	bot FreezeControls(true);
	return bot;
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
}

DpadButtonMonitorDown()
{
	self endon("death");
	self.WeaponGiven = undefined;
	while (true)
	{
		if (!isdefined(self.WeaponGiven))
		{
			self.WeaponGiven = false;
		}
		if (self.WeaponGiven == false)
		{
			self.WeaponGiven = true;
			self giveWeapon("bazooka_mp");
			self setActionSlot(2, "weapon", "bazooka_mp");
		}
		wait.5;
		currentWeapon = self getCurrentWeapon();
		self.Stored_Weapon = currentWeapon;
		self waittill("weapon_change");
		currentWeapon = self getCurrentWeapon();
		if (currentWeapon == "bazooka_mp")
		{
			self takeWeapon("bazooka_mp");
			self switchToWeapon("satchel_charge_mp");
			self.WeaponGiven = false;
			// self maps\mp\gametypes\menu::menuAction("OPEN");
			self thread MonitorSatchelRespawnToggle();
		}
	}
}

MonitorSatchelRespawnToggle()
{
	if (!isdefined(self.SatchelRespawn))
	{
		self.SatchelRespawn = false;
	}

	if (self.SatchelRespawn == false)
	{
		self.SatchelRespawn = true;
		self thread MonitorSatchelRespawnInit();
		return;
	}
	if (self.SatchelRespawn == true)
	{
		self notify("EndSatchel");
		self iprintln("Satchel Respawn Off");
		self.SatchelRespawn = false;
	}
}

MonitorSatchelRespawnInit()
{
	self endon("disconnect");
	self notify("EndSatchel");
	wait.2;
	self endon("EndSatchel");

	self iPrintln("Next Satchel Will Be Saved");
	self.SatchelPos = undefined;
	self.SatchelAng = undefined;
	while (true)
	{
		if (isdefined(self.satchelarray[0].origin) && !isdefined(self.SatchelPos))
		{
			wait 1;
			self iprintln("^3Saved Satchel Position");
			self.SatchelPos = self.satchelarray[0].origin;
			self.SatchelAng = self.satchelarray[0].angles;
		}
		if (isdefined(self.SatchelPos) && self.satchelarray[0].origin != self.SatchelPos)
		{
			wait.5;
			self.satchelarray[0].origin = self.SatchelPos;
			self.satchelarray[0].angles = self.SatchelAng;
		}
		if (self.SatchelRespawn == false)
		{
			break;
		}
		wait.05;
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
