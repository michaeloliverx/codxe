// #include common_scripts\utility;
// #include maps\mp\gametypes\_hud_util;

init()
{
	setDvar("scr_game_forceuav", 0); // Disable compass

	setDvar("player_sprintUnlimited", 1);	// Unlimited sprint
	setDvar("jump_slowdownEnable", 0);	// Disable jump slowdown

	setDvar("bg_fallDamageMaxHeight", 9999);	// Disable fall damage
	setDvar("bg_fallDamageMinHeight", 9998);	// Disable fall damage

	setDvar("scr_war_timelimit", 0);	// Disable time limit

	setDvar("scr_hardpoint_allowuav", 0);	// Disable UAV hardpoint and waypoint

	setDvar("sv_cheats", 1);	// Enable cheats

	level thread onPlayerConnect();
}

onPlayerConnect()
{
	for (;;)
	{
		level waittill("connecting", player);

		player thread onPlayerSpawned();
	}
}

onPlayerSpawned()
{
	self endon("disconnect");
	for (;;)
	{
		self waittill("spawned_player");

		self thread cj_setup_loadout();
		self thread replenish_ammo();
		self thread watch_buttons();
		self thread set_client_dvars();

		self Cbuf_ExecuteBuffer("bind BUTTON_LSHLDR toggle pm_fixed_fps 333 250 125");
		self Cbuf_ExecuteBuffer("bind BUTTON_RSHLDR toggle pm_fixed_fps 125 250 333");
		self Cbuf_ExecuteBuffer("bind DPAD_DOWN +frag");
		self Cbuf_ExecuteBuffer("bind DPAD_RIGHT +smoke");

		self Cbuf_ExecuteBuffer("bind DPAD_RIGHT +smoke");
	}
}

cj_setup_loadout()
{
	self TakeAllWeapons();

	wait 0.05;

	// self GiveWeapon("deserteagle_mp");
	self Cbuf_ExecuteBuffer("give deserteagle_mp");
	self GiveWeapon("mp5_mp");
	self GiveWeapon("rpg_mp");
	self SetActionSlot(3, "weapon", "rpg_mp");

	wait 0.05;
	self SwitchToWeapon("deserteagle_mp");
}

set_client_dvars()
{
	self endon("disconnect");

	self setClientDvar("fx_enable", 0);			// Disable effects
	self setClientDvar("ui_hud_hardcore", 1);	// Hardcore HUD
	self setClientDvar("cg_drawCrosshair", 0); // Disable crosshair

	// FPS
	// self setClientDvar("r_vsync", 0);
	// self setClientDvar("cg_drawFPS", 1);
	// self setClientDvar("com_maxfps", 333);

	// Fixed FPS
	self setClientDvar("pm_fixed_fps_enable", 1);
	self setClientDvar("pm_fixed_fps", 250);

	self setClientDvar("cg_fov", 70);

	self setClientDvar("cg_draw2D", 0); // Disable 2D HUD
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

watch_buttons()
{
	self endon("end_respawn");
	self endon("disconnect");

	for (;;)
	{

		if (self button_pressed("frag"))
		{
			self toggle_ufo();
			wait 0.25;
		}
		else if (self button_pressed_twice("melee"))
		{
			self savePos(0);
			wait 0.25;
		}
		else if (self button_pressed("smoke"))
		{
			self loadPos(0);
			wait 0.25;
		}

		wait 0.05;
	}
}

/**
 * Check if a button is pressed.
 */
button_pressed(button)
{
	switch (ToLower(button))
	{
	case "frag":
		return self fragbuttonpressed();
	case "melee":
		return self meleebuttonpressed();
	case "smoke":
		return self secondaryoffhandbuttonpressed();
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

	wait 0.05;
	self freezecontrols(false);
}

toggle_ufo()
{
	if (self.sessionstate == "playing")
	{
		self allowSpectateTeam("freelook", true);
		self.sessionstate = "spectator";
	}
	else
	{
		self allowSpectateTeam("freelook", false);
		self.sessionstate = "playing";
	}
}
