#include maps\codxe_utility;

init()
{

	level.start_point = "no_game"; // Starts what appears to be a dev mode game with no objectives/AI
	// SetSavedDvar("ufoHitsTriggers", "0"); // ? Investigate side effects of this

	level.player thread setup_player();
}

setup_player()
{
	// Prevents multiple calls to this function
	if (isdefined(self.cj_setup_player))
		return;

	self.cj_setup_player = true;

	wait 3;

	self hud_hide_all();

	self unlock_all_missions();

	self god();

	self setup_loadout();

	self setup_dvars();

	self thread watch_player_commands();
	self thread replenish_ammo();
	self thread rpg_switch();
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
	wait(0.05);
	// HUD
	SetSavedDvar("compass", 0);
	SetSavedDvar("ammoCounterHide", 1);
	SetSavedDvar("hud_showStance", 0);
	SetSavedDvar("hud_drawHUD", 0);
	SetSavedDvar("hud_dpad_arrow", 0);
	SetSavedDvar("cg_drawCrosshair", 0);
	SetSavedDvar("actionSlotsHide", 1);

	SetSavedDvar("cg_fov", 70);

	SetSavedDvar("loc_warnings", 0);
}

watch_player_commands()
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
		wait 0.1;
	}
}

/**
 * Check if a button is pressed.
 */
button_pressed(button)
{
	switch (ToLower(button))
	{
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
