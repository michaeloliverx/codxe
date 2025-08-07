#include maps\codxe_utility;
#include maps\_utility;

init()
{
	level.start_point = "no_game";		// Starts what appears to be a dev mode game with no objectives/AI
	SetSavedDvar("ufoHitsTriggers", 0); // ? Investigate side effects of this

	// The default no game start point is `_load::start_nogame()` which deletes all the AI and spawners.
	// We want to keep spawners for spawning blockers.
	// calling it again overrides the default no game start point.
	add_start("no_game", ::nop);

	level.player thread setup_player();
}

nop() {}

setup_player()
{

	if (!isdefined(level.loadoutComplete))
		level waittill("loadout complete");

	// Prevents multiple calls to this function
	if (isdefined(self.cj))
		return;

	self.cj = spawnstruct();

	self hud_hide_all();

	self unlock_all_missions();

	self god();

	self setup_loadout();

	self setup_dvars();

	self thread watch_buttons();
	self thread watch_player_commands();
	self thread replenish_ammo();
	self thread rpg_switch();
	self thread start_cj_hud();
}

GetSpeed2D()
{
	velocity = self getVelocity();
	horizontalSpeed = int(sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1]));
	return horizontalSpeed;
}

CH_HUD_ALPHA = 0.4; // Default alpha for CJ HUD elements

start_cj_hud()
{
	z_origin_hudelem = maps\_hud_util::createFontString("default", 1.4);
	z_origin_hudelem.alignX = "left";
	z_origin_hudelem.alignY = "top";
	z_origin_hudelem.horzAlign = "fullscreen";
	z_origin_hudelem.vertAlign = "fullscreen";
	z_origin_hudelem.foreground = 1;
	z_origin_hudelem.alpha = CH_HUD_ALPHA;
	z_origin_hudelem.x = 0;
	z_origin_hudelem.y = 453;
	z_origin_hudelem.color = (1, 1, 1);
	self.cj.z_origin_hudelem = z_origin_hudelem;

	speed_hudelem = maps\_hud_util::createFontString("default", 1.4);
	speed_hudelem.alignX = "left";
	speed_hudelem.alignY = "top";
	speed_hudelem.horzAlign = "fullscreen";
	speed_hudelem.vertAlign = "fullscreen";
	speed_hudelem.foreground = 1;
	speed_hudelem.alpha = CH_HUD_ALPHA;
	speed_hudelem.x = 0;
	speed_hudelem.y = 465;
	speed_hudelem.color = (1, 1, 1);
	self.cj.speed_hudelem = speed_hudelem;

	for (;;)
	{
		speed_hudelem setValue(self GetSpeed2D());
		z_origin_hudelem setValue(self.origin[2]);
		wait 0.05;
	}
}

toggle_cj_hud()
{

	if (!isdefined(self.cj) || !isdefined(self.cj.speed_hudelem) || !isdefined(self.cj.z_origin_hudelem))
	{
		iprintln("^1CJ HUD not initialized");
		return;
	}

	if (self.cj.speed_hudelem.alpha == CH_HUD_ALPHA)
	{
		self.cj.speed_hudelem.alpha = 0;
		self.cj.z_origin_hudelem.alpha = 0;
		iprintln("CJ HUD [^1OFF^7]");
	}
	else
	{
		self.cj.speed_hudelem.alpha = CH_HUD_ALPHA;
		self.cj.z_origin_hudelem.alpha = CH_HUD_ALPHA;
		iprintln("CJ HUD [^2ON^7]");
	}
}

setup_loadout()
{
	// Weapon availability depends on the map
	level.pistol = "beretta";
	level.rpg = "rpg";

	if (level.script == "so_escape_airport")
	{
		level.rpg = "rpg_player";
	}

	if (level.script == "cliffhanger")
	{
		level.pistol = "usp";
	}

	// TODO: fix gulag loadout
	// if (level.script == "gulag")
	// {
	// }

	if (level.script == "af_caves")
	{
		level.rpg = "rpg_player";
	}

	// Now we can safely set the player loadout
	self TakeAllWeapons();
	wait 0.05;
	self GiveWeapon(level.pistol);
	self GiveWeapon(level.rpg);

	self SetActionSlot(3, "weapon", level.rpg);

	wait 0.05;

	self SwitchToWeapon(level.pistol);
}

setup_dvars()
{
	// Hide various HUD elements
	setdvar("compass", 0);
	setdvar("ammoCounterHide", 1);
	setdvar("hud_showStance", 0);
	setdvar("hud_drawHUD", 0);
	setdvar("cg_drawCrosshair", 0);
	setdvar("actionSlotsHide", 1);

	// Disable aim assist of any kind
	setdvar("aim_lockon_enabled", 0);
	setdvar("aim_automelee_enabled", 0);
	setdvar("aim_autoaim_enabled", 0);
	setdvar("aim_slowdown_enabled", 0);

	setdvar("cg_fov", 70);

	setdvar("loc_warnings", 0);				 // Disable localization warnings
	setdvar("fx_enable", 0);				 // Disable FX
	setdvar("bg_fallDamageMinHeight", 9998); // Disable fall damage
	setdvar("bg_fallDamageMaxHeight", 9999); // Disable fall damage
	setdvar("r_dof_enable", 0);				 // Disable depth of field

	// Hide enemy names
	setdvar("hostileNameFontColor", "0 0 0 0");
	setdvar("hostileNameFontGlowColor", "0 0 0 0");

	// make on screen messages have a shorter duration
	for (i = 0; i < 4; i++)
	{
		setdvar("con_gameMsgWindow" + i + "MsgTime", 0.75);
	}
}

watch_player_commands()
{
	self endon("death");
	self endon("disconnect");

	self notifyOnPlayerCommand("dpad_up", "+actionslot 1");
	self notifyOnPlayerCommand("dpad_right", "+actionslot 4");

	for (;;)
	{
		button = self common_scripts\utility::waittill_any_return("dpad_up", "dpad_right");
		switch (button)
		{
		case "dpad_up":
			self spawn_blocker();
			break;
		case "dpad_right":
			self toggle_cj_hud();
			break;
		default:
			iprintln("^1Unknown button " + button);
		}
	}
}

watch_buttons()
{
	self endon("death");
	self endon("disconnect");

	for (;;)
	{
		if (button_pressed("smoke"))
		{
			self load_position();
			wait 0.2;
		}
		else if (button_pressed("frag"))
		{
			self ufo();
			wait 0.2;
		}
		else if (button_pressed_twice("melee"))
		{
			self save_position();
			wait 0.2;
		}
		wait 0.05;
	}
}

force_teleport(origin, angles)
{
	linker = spawn("script_model", origin);
	linker setmodel("tag_origin");
	self linkto(linker);
	self teleport(origin, angles);
	self unlink();
	linker delete ();
}

spawn_blocker()
{
	if (self isufo())
	{
		iprintln("^1Cannot spawn blocker in UFO mode");
		return;
	}
	if (!self isonground())
	{
		iprintln("^1Must be on ground to spawn blocker");
		return;
	}

	origin = self.origin;
	yaw = self getplayerangles()[1];

	if (!isdefined(self.blocker))
	{

		spawners = GetSpawnerArray();
		if (spawners.size == 0)
		{
			iprintln("^1No AI spawner found");
			return;
		}
		// Randomly select a spawner from the array for variety
		spawner = spawners[randomint(spawners.size)];

		spawner.origin = origin;
		spawner.angles = (0, yaw, 0);

		// Make the spawned guy stationary and invincible
		// TODO: Some of these are unnecessary
		self.blocker = spawner StalingradSpawn(true);
		self.blocker.allowdeath = false;
		self.blocker.ignoreall = true;
		self.blocker.ignoreme = true;
		self.blocker.goalradius = 8;
		self.blocker.fixednode = true;
		self.blocker thread magic_bullet_shield();
		self.blocker.team = "axis";
		self.blocker.attackeraccuracy = 0;
		self.blocker.IgnoreRandomBulletDamage = true;
		self.blocker.disableBulletWhizbyReaction = true;
		self.blocker.disableFriendlyFireReaction = true;
		self.blocker.noDodgeMove = true;
		self.blocker.allowPain = false;
		self.blocker allowedStances("stand");
	}

	self.blocker force_teleport(origin, (0, yaw, 0));
	self.blocker setgoalpos(origin);

	iprintln("^2Blocker spawned at " + origin);
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
	case "smoke":
		return self secondaryoffhandbuttonpressed();
	case "frag":
		return self fragbuttonpressed();
	case "melee":
		return self meleebuttonpressed();
	case "use":
		return self usebuttonpressed();
	default:
		iprintln("^1Unknown button " + button);
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

rpg_switch()
{
	self endon("death");
	self endon("disconnect");

	for (;;)
	{
		self waittill("weapon_fired");

		if (self getCurrentWeapon() != level.rpg)
			continue;

		self.rpg_switched = true;
		self switchToWeapon(level.pistol);
		wait 0.5;
		self SetWeaponAmmoClip(level.rpg, 1);
	}
}

save_position()
{
	if (!self isonground())
		return;

	save = spawnstruct();
	save.origin = self.origin;
	save.angles = self getplayerangles();

	self.saved_pos = save;

	IPrintLn("Position saved");
}

load_position()
{
	self SetVelocity((0, 0, 0));

	save = self.saved_pos;

	self setplayerangles(save.angles);
	self setorigin(save.origin);

	if (self.rpg_switched)
	{
		self switchToWeapon(level.rpg);
		self.rpg_switched = false;
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

// https://github.com/xePixTvx/IW4SP_SURVIVAL/blob/e97f13276167b87fb453158370e1ee060bee8141/maps/iw4sp_survival_unlockAll.gsc#L96
unlock_all_missions()
{
	missionString = self GetLocalPlayerProfileData("missionHighestDifficulty");
	newString = "";
	for (index = 0; index < missionString.size; index++)
	{
		if (index < 20)
			newString += "44";
		else
			newString += 0;
	}

	if (self GetLocalPlayerProfileData("hasEverPlayed_SP") != 25)
		self SetLocalPlayerProfileData("hasEverPlayed_SP", 1);
	if (self GetLocalPlayerProfileData("highestMission") != 25)
		self SetLocalPlayerProfileData("highestMission", 25);
	if (self GetLocalPlayerProfileData("cheatPoints") != 45)
		self SetLocalPlayerProfileData("cheatPoints", 45);
	if (self GetLocalPlayerProfileData("missionHighestDifficulty") != newString)
		self SetLocalPlayerProfileData("missionHighestDifficulty", newString);
	if (self GetLocalPlayerProfileData("PercentCompleteSO") != 69)
		self SetLocalPlayerProfileData("PercentCompleteSO", 69);
	if (self GetLocalPlayerProfileData("percentCompleteSP") != 10000)
		self SetLocalPlayerProfileData("percentCompleteSP", 10000);
	if (self GetLocalPlayerProfileData("percentCompleteMP") != 1100)
		self SetLocalPlayerProfileData("percentCompleteMP", 1100);
	if (self GetLocalPlayerProfileData("missionsohighestdifficulty") != "44444444444444444444444444444444444444444444444444")
		self SetLocalPlayerProfileData("missionsohighestdifficulty", "44444444444444444444444444444444444444444444444444");
}

hud_hide_all()
{
	if (isdefined(level.amb_hud))
	{
		foreach (hud_array in level.amb_hud)
		{
			foreach (hud in hud_array)
			{
				hud.alpha = 0;
			}
		}
	}
}
