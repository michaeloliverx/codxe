/*
	Enemy / AI mods. Spawning reuses the level's own spawners, so anything spawned is an actor
	type the map already loaded (Imperial soldiers, Wehrmacht, zombies, hellhounds...).
*/

#include maps\_utility;
#include common_scripts\utility;
#include maps\mod_menu\util;

// ---------------------------------------------------------------------------
// Mass actions
// ---------------------------------------------------------------------------

kill_all()
{
	enemies = mm_enemies();
	for (i = 0; i < enemies.size; i++)
		mm_kill(enemies[i], self);
	self iprintln("Killed ^3" + enemies.size + "^7 enemies");
}

gib_all()
{
	enemies = mm_enemies();
	for (i = 0; i < enemies.size; i++)
	{
		if (!isAlive(enemies[i]))
			continue;
		mm_gib(enemies[i], mm_random_gib_ref());
		mm_blood(enemies[i].origin + (0, 0, 40));
	}
	wait 0.1;
	for (i = 0; i < enemies.size; i++)
		mm_kill(enemies[i], self);
	self iprintln("^1Gibbed ^7" + enemies.size + " enemies");
}

launch_all()
{
	enemies = mm_enemies();
	for (i = 0; i < enemies.size; i++)
		thread mm_fling(enemies[i], mm_random_vec(250) + (0, 0, 900), self);
	self iprintln("^5Houston, we have liftoff");
}

teleport_enemies_here()
{
	pos = self mm_aim_pos();
	enemies = mm_enemies();
	for (i = 0; i < enemies.size; i++)
	{
		if (isAlive(enemies[i]))
			enemies[i] teleport(mm_ground(pos + (randomFloatRange(-120, 120), randomFloatRange(-120, 120), 32)));
	}
	self iprintln("Summoned ^3" + enemies.size + "^7 enemies to your crosshair");
}

switch_sides()
{
	enemies = mm_enemies();
	count = 0;
	for (i = 0; i < enemies.size; i++)
	{
		if (!isAlive(enemies[i]) || enemies[i] is_zombie_actor())
			continue;
		enemies[i].team = "allies";
		count++;
	}
	self iprintlnbold("^2" + count + " enemies defected to your side!");
}

// Half the enemies turn on the other half.
civil_war()
{
	enemies = mm_enemies();
	count = 0;
	for (i = 0; i < enemies.size; i += 2)
	{
		if (!isAlive(enemies[i]) || enemies[i] is_zombie_actor())
			continue;
		enemies[i].team = "allies";
		enemies[i].ignoreme = true;
		count++;
	}
	self iprintlnbold("^1CIVIL WAR ^7- " + count + " traitors");
}

is_zombie_actor()
{
	return isDefined(self.animname) && (self.animname == "zombie" || self.animname == "zombie_dog");
}

// ---------------------------------------------------------------------------
// Looping modifiers
// ---------------------------------------------------------------------------

freeze_set(on)
{
	level notify("mm_stop_freeze");
	self mm_onoff("Freeze Enemies", on);
	enemies = mm_enemies();
	if (!on)
	{
		for (i = 0; i < enemies.size; i++)
			enemies[i] unfreeze_ai();
		return;
	}

	level endon("mm_stop_freeze");
	for (;;)
	{
		enemies = mm_enemies();
		for (i = 0; i < enemies.size; i++)
		{
			if (isAlive(enemies[i]) && !isDefined(enemies[i].mm_freezer))
				enemies[i] freeze_ai();
		}
		wait 0.5;
	}
}

freeze_ai()
{
	self.mm_freezer = mm_spawn_mover(self.origin, self.angles);
	self linkTo(self.mm_freezer);
	self.mm_old_ignoreall = self.ignoreall;
	self.ignoreall = true;
	self thread freeze_cleanup();
}

unfreeze_ai()
{
	if (!isDefined(self.mm_freezer))
		return;
	self unlink();
	if (isDefined(self.mm_old_ignoreall))
		self.ignoreall = self.mm_old_ignoreall;
	self.mm_freezer delete();
	self.mm_freezer = undefined;
}

freeze_cleanup()
{
	mover = self.mm_freezer;
	self waittill("death");
	if (isDefined(mover))
		mover delete();
}

pacifist_set(on)
{
	level notify("mm_stop_pacifist");
	self mm_onoff("Pacifist Enemies", on);
	if (!on)
	{
		enemies = mm_enemies();
		for (i = 0; i < enemies.size; i++)
		{
			if (isDefined(enemies[i].mm_pacified))
			{
				enemies[i].ignoreall = false;
				enemies[i].pacifist = false;
				enemies[i].mm_pacified = undefined;
			}
		}
		return;
	}

	level endon("mm_stop_pacifist");
	for (;;)
	{
		enemies = mm_enemies();
		for (i = 0; i < enemies.size; i++)
		{
			if (!isAlive(enemies[i]) || enemies[i] is_zombie_actor())
				continue;
			enemies[i].ignoreall = true;
			enemies[i].pacifist = true;
			enemies[i].mm_pacified = true;
		}
		wait 0.5;
	}
}

one_hit_set(on)
{
	level notify("mm_stop_onehit");
	self mm_onoff("One Hit Kills", on);
	if (!on)
		return;

	level endon("mm_stop_onehit");
	for (;;)
	{
		enemies = mm_enemies();
		for (i = 0; i < enemies.size; i++)
		{
			if (isAlive(enemies[i]) && enemies[i].health > 1)
				enemies[i].health = 1;
		}
		wait 0.25;
	}
}

speed_set(value)
{
	level notify("mm_stop_ai_speed");
	level endon("mm_stop_ai_speed");
	for (;;)
	{
		enemies = mm_enemies();
		for (i = 0; i < enemies.size; i++)
			enemies[i].moveplaybackrate = value;
		if (value == 1)
			return;
		wait 0.5;
	}
}

stormtrooper_set(on)
{
	level notify("mm_stop_accuracy");
	self mm_onoff("Stormtrooper Aim", on);
	if (!on)
	{
		enemies = mm_enemies();
		for (i = 0; i < enemies.size; i++)
			enemies[i].baseaccuracy = 1;
		return;
	}

	level endon("mm_stop_accuracy");
	for (;;)
	{
		enemies = mm_enemies();
		for (i = 0; i < enemies.size; i++)
			enemies[i].baseaccuracy = 0;
		wait 0.5;
	}
}

friendly_god_set(on)
{
	level notify("mm_stop_friendly_god");
	self mm_onoff("Invincible Squad", on);
	if (!on)
		return;

	level endon("mm_stop_friendly_god");
	for (;;)
	{
		allies = getAIArray("allies");
		for (i = 0; i < allies.size; i++)
		{
			if (isAlive(allies[i]) && allies[i].health < 100000)
			{
				allies[i].maxhealth = 100000;
				allies[i].health = 100000;
			}
		}
		wait 0.5;
	}
}

// ---------------------------------------------------------------------------
// Spawning
// ---------------------------------------------------------------------------

// One spawner per actor classname, e.g. actor_axis_jap_reg_type99rifle.
spawner_types()
{
	result = [];
	seen = [];
	spawners = getSpawnerArray();
	for (i = 0; i < spawners.size && result.size < 40; i++)
	{
		name = spawners[i].classname;
		if (isDefined(seen[name]))
			continue;
		seen[name] = true;
		result[result.size] = spawners[i];
	}
	return result;
}

spawner_label(spawner)
{
	name = spawner.classname;
	if (isSubStr(name, "actor_"))
		name = getSubStr(name, 6, name.size);
	return mm_short(name, 22);
}

spawn_from(spawner, pos)
{
	if (!isDefined(spawner))
		return undefined;

	oldOrigin = spawner.origin;
	spawner.origin = pos;
	spawner.count = 1;
	guy = spawner stalingradSpawn();
	spawner.origin = oldOrigin;

	if (spawn_failed(guy))
		return undefined;

	guy teleport(pos);
	return guy;
}

spawn_type_at_crosshair(spawner)
{
	guy = spawn_from(spawner, self mm_aim_pos());
	if (isDefined(guy))
		self iprintln("Spawned ^3" + spawner_label(spawner));
	else
		self iprintln("^1Spawn failed (AI limit reached?)");
}

spawn_random_enemy_at(pos)
{
	types = spawner_types();
	if (types.size == 0)
		return undefined;
	return spawn_from(types[randomInt(types.size)], pos);
}

// Spawns an AI, flips it to your team and keeps it at your side.
bodyguard()
{
	self endon("disconnect");
	if (mm_is_zombies())
	{
		self iprintln("^1Zombies do not make loyal bodyguards");
		return;
	}

	types = spawner_types();
	if (types.size == 0)
	{
		self iprintln("^1No spawners on this map");
		return;
	}

	guy = spawn_from(types[randomInt(types.size)], mm_ground(self.origin + self mm_forward() * 80 + (0, 0, 32)));
	if (!isDefined(guy))
	{
		self iprintln("^1Spawn failed");
		return;
	}

	guy.team = "allies";
	guy.maxhealth = 100000;
	guy.health = 100000;
	guy.goalradius = 160;
	self iprintlnbold("^2Bodyguard reporting for duty");

	while (isAlive(guy))
	{
		guy setGoalPos(self.origin);
		wait 1;
	}
}
