/*
	Chaos Mode: a random event every N seconds. Timed events undo themselves.
*/

#include maps\_utility;
#include common_scripts\utility;
#include maps\mod_menu\util;

chaos_set(on)
{
	level notify("mm_stop_chaos");
	self mm_onoff("Chaos Mode", on);
	if (!on)
		return;

	level endon("mm_stop_chaos");
	mm_announce("^1CHAOS MODE ^7ENGAGED");
	last = -1;
	for (;;)
	{
		wait mm_get("chaos_interval", "level", 20);
		index = randomInt(16);
		if (index == last)
			index = (index + 1) % 16;
		last = index;
		self thread chaos_event(index);
	}
}

chaos_interval_set(value)
{
	self iprintln("Chaos every ^3" + value + "^7 seconds");
}

chaos_event(index)
{
	self endon("disconnect");
	switch (index)
	{
	case 0:
		mm_announce("^5CHAOS: ^7Moon gravity");
		mm_set_dvar("g_gravity", "120");
		mm_set_dvar("phys_gravity", "-150");
		wait 15;
		mm_set_dvar("g_gravity", mm_get("gravity", "level", 800));
		mm_restore_dvar("phys_gravity", "-800");
		break;
	case 1:
		mm_announce("^5CHAOS: ^7Bullet time");
		setTimeScale(0.4);
		wait 5;
		setTimeScale(mm_get("timescale", "level", 1));
		break;
	case 2:
		mm_announce("^5CHAOS: ^7Fast forward");
		setTimeScale(1.8);
		wait 8;
		setTimeScale(mm_get("timescale", "level", 1));
		break;
	case 3:
		visions = maps\mod_menu\world::vision_names();
		mm_announce("^5CHAOS: ^7Everything looks wrong");
		visionSetNaked(visions[randomIntRange(1, visions.size)], 1);
		wait 15;
		visionSetNaked(mm_level_vision(), 2);
		break;
	case 4:
		self maps\mod_menu\fun::meteor_shower();
		break;
	case 5:
		self maps\mod_menu\fun::rapture();
		break;
	case 6:
		mm_announce("^5CHAOS: ^7Everybody gets gibbed");
		self maps\mod_menu\ai::gib_all();
		break;
	case 7:
		mm_announce("^5CHAOS: ^7Earthquake");
		earthquake(0.6, 8, self.origin, 100000);
		break;
	case 8:
		self maps\mod_menu\fun::blood_rain();
		break;
	case 9:
		mm_announce("^5CHAOS: ^7Disco inferno");
		self thread maps\mod_menu\fun::disco_set(true);
		wait 12;
		level notify("mm_stop_disco");
		resetSunLight();
		visionSetNaked(mm_level_vision(), 1);
		break;
	case 10:
		mm_announce("^5CHAOS: ^7Weapon roulette");
		players = get_players();
		for (i = 0; i < players.size; i++)
			players[i] maps\mod_menu\weapons::give_random_weapon();
		break;
	case 11:
		mm_announce("^5CHAOS: ^7Sonic speed");
		players = get_players();
		for (i = 0; i < players.size; i++)
			players[i] setMoveSpeedScale(3);
		wait 12;
		for (i = 0; i < players.size; i++)
		{
			if (isDefined(players[i]))
				players[i] setMoveSpeedScale(players[i] mm_get("speed", "self", 1));
		}
		break;
	case 12:
		mm_announce("^5CHAOS: ^7Silent film");
		setDvar("sf_use_chaplin", 1);
		wait 15;
		setDvar("sf_use_chaplin", 0);
		break;
	case 13:
		mm_announce("^5CHAOS: ^7Hurricane");
		mm_set_dvar("wind_global_vector", "3000 -2500 0");
		wait 15;
		mm_restore_dvar("wind_global_vector", "0 0 0");
		break;
	case 14:
		mm_announce("^5CHAOS: ^7Upside-down physics");
		mm_set_dvar("phys_gravity", "500");
		physicsExplosionSphere(self.origin, 2000, 100, 1);
		wait 12;
		mm_restore_dvar("phys_gravity", "-800");
		break;
	default:
		mm_announce("^5CHAOS: ^7Enemies go flying");
		self maps\mod_menu\ai::launch_all();
		break;
	}
}
