/*
	World, physics and visuals. Also exposes two stock WaW systems that are normally hidden:
	- The CoD4-era "special features" cheats in maps\_cheat.gsc, which poll sf_use_* dvars.
	- The campaign Death Card collectibles (maps\_collectibles.gsc), toggled live with
	  SetCollectible()/UnsetCollectible().
*/

#include maps\_utility;
#include common_scripts\utility;
#include maps\mod_menu\util;

// ---------------------------------------------------------------------------
// Time & gravity
// ---------------------------------------------------------------------------

timescale_set(value)
{
	setTimeScale(value);
}

gravity_set(value)
{
	mm_set_dvar("g_gravity", value);
}

phys_gravity_names()
{
	names = [];
	names[names.size] = "Normal";
	names[names.size] = "Moon";
	names[names.size] = "Zero-G";
	names[names.size] = "Upside Down";
	return names;
}

phys_gravity_set(index)
{
	values = [];
	values[0] = "-800";
	values[1] = "-150";
	values[2] = "0";
	values[3] = "400";
	if (index == 0)
		mm_restore_dvar("phys_gravity", values[0]);
	else
		mm_set_dvar("phys_gravity", values[index]);
	names = phys_gravity_names();
	self iprintln("Physics gravity: ^3" + names[index]);
}

wind_set(on)
{
	if (on)
	{
		mm_set_dvar("wind_global_vector", "2500 1800 0");
		mm_set_dvar("wind_global_low_strength_percent", "1");
	}
	else
	{
		mm_restore_dvar("wind_global_vector", "0 0 0");
		mm_restore_dvar("wind_global_low_strength_percent", "0.5");
	}
	self mm_onoff("Hurricane Winds", on);
}

// ---------------------------------------------------------------------------
// Fog, sun & vision
// ---------------------------------------------------------------------------

fog_names()
{
	names = [];
	names[names.size] = "Unchanged";
	names[names.size] = "Crystal Clear";
	names[names.size] = "Blood Moon";
	names[names.size] = "Toxic";
	names[names.size] = "Silent Hill";
	names[names.size] = "Midnight";
	names[names.size] = "Bubblegum";
	return names;
}

fog_set(index)
{
	switch (index)
	{
	case 1:
		setVolFog(20000, 40000, 2000, 0, 0.6, 0.7, 0.8, 2);
		break;
	case 2:
		setVolFog(100, 900, 600, 0, 0.55, 0, 0, 2);
		break;
	case 3:
		setVolFog(80, 700, 400, 0, 0.2, 0.6, 0.05, 2);
		break;
	case 4:
		setVolFog(0, 250, 2000, 0, 0.55, 0.55, 0.55, 2);
		break;
	case 5:
		setVolFog(0, 450, 1000, 0, 0, 0, 0.02, 2);
		break;
	case 6:
		setVolFog(50, 800, 600, 0, 1, 0.4, 0.7, 2);
		break;
	default:
		self iprintln("Fog changes last until the level restarts");
		return;
	}
	names = fog_names();
	self iprintln("Fog: ^3" + names[index]);
}

sun_names()
{
	names = [];
	names[names.size] = "Default";
	names[names.size] = "Blackout";
	names[names.size] = "Blood Red";
	names[names.size] = "Radioactive";
	names[names.size] = "Deep Blue";
	names[names.size] = "Supernova";
	return names;
}

sun_set(index)
{
	switch (index)
	{
	case 1:
		setSunLight(0, 0, 0);
		break;
	case 2:
		setSunLight(3, 0, 0);
		break;
	case 3:
		setSunLight(0.5, 3, 0.2);
		break;
	case 4:
		setSunLight(0.1, 0.3, 3);
		break;
	case 5:
		setSunLight(6, 5, 4);
		break;
	default:
		resetSunLight();
		break;
	}
}

// Vision files that ship with every singleplayer level: the level's own set plus the ones
// maps\_cheat.gsc switches between. Missing files only print a console warning.
vision_names()
{
	names = [];
	names[names.size] = mm_level_vision();
	names[names.size] = "cheat_bw";
	names[names.size] = "cheat_invert";
	names[names.size] = "cheat_contrast";
	names[names.size] = "cheat_bw_invert";
	names[names.size] = "cheat_bw_contrast";
	names[names.size] = "cheat_invert_contrast";
	names[names.size] = "cheat_bw_invert_contrast";
	names[names.size] = "sepia";
	if (mm_is_zombies())
	{
		names[names.size] = "zombie_turned";
		names[names.size] = "zombie_death";
		names[names.size] = "laststand";
	}
	return names;
}

vision_set(name)
{
	visionSetNaked(name, 1);
	self iprintln("Vision: ^3" + name);
}

// ---------------------------------------------------------------------------
// Visual toggles
// ---------------------------------------------------------------------------

fullbright_set(on)
{
	self setClientDvar("r_fullbright", on);
	self mm_onoff("Fullbright", on);
}

motion_blur_set(on)
{
	if (on)
	{
		mm_set_dvar("r_motionblur_enable", "1");
		mm_set_dvar("r_motionblur_positionFactor", "2");
		mm_set_dvar("r_motionblur_directionFactor", "2");
	}
	else
	{
		mm_restore_dvar("r_motionblur_enable", "0");
		mm_restore_dvar("r_motionblur_positionFactor", "0.1");
		mm_restore_dvar("r_motionblur_directionFactor", "0.1");
	}
	self mm_onoff("Extreme Motion Blur", on);
}

hide_hud_set(on)
{
	if (on)
		mm_set_dvar("hud_drawhud", "0");
	else
		mm_set_dvar("hud_drawhud", "1");
	self mm_onoff("Hide HUD", on);
}

hide_gun_set(on)
{
	self setClientDvar("cg_drawGun", !on);
	self mm_onoff("Hide Gun", on);
}

double_vision_set(on)
{
	if (on)
		self setDoubleVision(6, 1);
	else
		self setDoubleVision(0, 1);
	self mm_onoff("Double Vision", on);
}

// ---------------------------------------------------------------------------
// maps\_cheat.gsc special features
// ---------------------------------------------------------------------------

cheat_set(on, dvar)
{
	setDvar(dvar, on);
}

cheat_bw_set(on)
{
	cheat_set(on, "sf_use_bw");
	self mm_onoff("Black & White", on);
}

cheat_invert_set(on)
{
	cheat_set(on, "sf_use_invert");
	self mm_onoff("Photo Negative", on);
}

cheat_contrast_set(on)
{
	cheat_set(on, "sf_use_contrast");
	self mm_onoff("Super Contrast", on);
}

cheat_chaplin_set(on)
{
	cheat_set(on, "sf_use_chaplin");
	self mm_onoff("Silent Film (Chaplin)", on);
}

cheat_slowmo_set(on)
{
	cheat_set(on, "sf_use_slowmo");
	self mm_onoff("Slow-mo Ability", on);
}

cheat_cluster_set(on)
{
	cheat_set(on, "sf_use_clustergrenade");
	self mm_onoff("Cluster Grenades", on);
}

cheat_ammo_set(on)
{
	cheat_set(on, "sf_use_ignoreammo");
	self mm_onoff("Bottomless Ammo (engine)", on);
}

// ---------------------------------------------------------------------------
// Death cards
// ---------------------------------------------------------------------------

card_get(name)
{
	if (!isDefined(level.collectible_unlocks) || !isDefined(level.collectible_unlocks[name]))
		return false;
	return maps\_collectibles::has_collectible(name);
}

card_toggle(name, label)
{
	if (!isDefined(level.collectible_unlocks) || !isDefined(level.collectible_unlocks[name]))
	{
		self iprintln("^1Death cards are not available on this map");
		return;
	}

	if (card_get(name))
	{
		unsetCollectible(name);
		self notify(name + "_end");
		self iprintln(label + " [^1OFF^7]");
		return;
	}

	setCollectible(name);
	self iprintln(label + " [^2ON^7]");

	switch (name)
	{
	case "collectible_vampire":
		self thread maps\_collectibles_game::vampire_main();
		break;
	case "collectible_berserker":
		self thread maps\_collectibles_game::berserker_main();
		break;
	case "collectible_sticksstones":
		self thread maps\_collectibles_game::sticksstones_main();
		break;
	default:
		break;
	}
}

card_thunder_get()
{
	return card_get("collectible_thunder");
}

card_thunder_set(on)
{
	card_toggle("collectible_thunder", "Thunder (explosive headshots)");
}

card_paintball_get()
{
	return card_get("collectible_paintball");
}

card_paintball_set(on)
{
	card_toggle("collectible_paintball", "Paintball");
}

card_dead_hands_get()
{
	return card_get("collectible_dead_hands");
}

card_dead_hands_set(on)
{
	card_toggle("collectible_dead_hands", "Cold Dead Hands");
}

card_zombie_get()
{
	return card_get("collectible_zombie");
}

card_zombie_set(on)
{
	card_toggle("collectible_zombie", "Undead");
}

card_hard_headed_get()
{
	return card_get("collectible_hard_headed");
}

card_hard_headed_set(on)
{
	card_toggle("collectible_hard_headed", "Hard Headed");
}

card_berserker_get()
{
	return card_get("collectible_berserker");
}

card_berserker_set(on)
{
	card_toggle("collectible_berserker", "Berserker");
}

card_vampire_get()
{
	return card_get("collectible_vampire");
}

card_vampire_set(on)
{
	card_toggle("collectible_vampire", "Vampire");
}

card_sticks_get()
{
	return card_get("collectible_sticksstones");
}

card_sticks_set(on)
{
	card_toggle("collectible_sticksstones", "Sticks and Stones");
}

card_flak_get()
{
	return card_get("collectible_flak_jacket");
}

card_flak_set(on)
{
	card_toggle("collectible_flak_jacket", "Flak Jacket");
}

card_armor_get()
{
	return card_get("collectible_body_armor");
}

card_armor_set(on)
{
	card_toggle("collectible_body_armor", "Body Armor");
}

card_morphine_get()
{
	return card_get("collectible_morphine");
}

card_morphine_set(on)
{
	card_toggle("collectible_morphine", "Morphine Shot");
}

card_harry_get()
{
	return card_get("collectible_dirtyharry");
}

card_harry_set(on)
{
	card_toggle("collectible_dirtyharry", "Dirty Harry");
}

card_hardcore_get()
{
	return card_get("collectible_hardcore");
}

card_hardcore_set(on)
{
	card_toggle("collectible_hardcore", "Hardcore");
}

// ---------------------------------------------------------------------------
// Destruction
// ---------------------------------------------------------------------------

destroy_destructibles()
{
	ents = getEntArray("destructible", "targetname");
	for (i = 0; i < ents.size; i++)
	{
		if (!isDefined(ents[i]))
			continue;
		setPlayerIgnoreRadiusDamage(true);
		radiusDamage(ents[i].origin + (0, 0, 24), 48, 5000, 4000);
		setPlayerIgnoreRadiusDamage(false);
		wait 0.1;
	}
	self iprintln("Detonated ^3" + ents.size + "^7 destructibles");
}

vehicles()
{
	return getEntArray("script_vehicle", "classname");
}

destroy_vehicles()
{
	list = vehicles();
	for (i = 0; i < list.size; i++)
	{
		if (!isDefined(list[i]))
			continue;
		mm_explode(list[i].origin + (0, 0, 40), 250, self);
		list[i] doDamage(100000, list[i].origin, self);
		wait 0.2;
	}
	self iprintln("Destroyed ^3" + list.size + "^7 vehicles");
}

vehicles_usable()
{
	list = vehicles();
	for (i = 0; i < list.size; i++)
		list[i] makeVehicleUsable();
	self iprintln("^3" + list.size + "^7 vehicles are now drivable - walk up and press X");
}
