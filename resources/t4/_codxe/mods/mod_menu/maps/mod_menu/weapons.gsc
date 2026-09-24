/*
	Weapons, bullet modifiers and aim toys.

	Only weapons the level has already loaded can be given, so the weapon list is built at runtime
	from the player's loadout, the level loadout, zombie box weapons and what the AI are carrying.
*/

#include maps\_utility;
#include common_scripts\utility;
#include maps\mod_menu\util;

// ---------------------------------------------------------------------------
// Weapon pool
// ---------------------------------------------------------------------------

weapon_pool()
{
	pool = [];

	owned = self getWeaponsList();
	for (i = 0; i < owned.size; i++)
		pool = mm_array_add_unique(pool, owned[i]);

	if (isDefined(level.player_loadout))
	{
		for (i = 0; i < level.player_loadout.size; i++)
			pool = mm_array_add_unique(pool, level.player_loadout[i]);
	}

	if (isDefined(level.zombie_weapons))
	{
		keys = getArrayKeys(level.zombie_weapons);
		for (i = 0; i < keys.size; i++)
			pool = mm_array_add_unique(pool, keys[i]);
	}

	if (isDefined(level.laststandpistol))
		pool = mm_array_add_unique(pool, level.laststandpistol);

	ai = getAIArray();
	for (i = 0; i < ai.size; i++)
	{
		if (isDefined(ai[i].weapon))
			pool = mm_array_add_unique(pool, ai[i].weapon);
		if (isDefined(ai[i].sidearm))
			pool = mm_array_add_unique(pool, ai[i].sidearm);
	}

	// Skip perk bottles and other script-only weapons.
	result = [];
	for (i = 0; i < pool.size; i++)
	{
		if (isSubStr(pool[i], "bottle") || isSubStr(pool[i], "syrette") || pool[i] == "zombie_melee")
			continue;
		result[result.size] = pool[i];
	}
	return result;
}

is_offhand(weapon)
{
	type = weaponType(weapon);
	return type == "grenade";
}

give_weapon(weapon)
{
	if (!self is_offhand(weapon) && !self hasWeapon(weapon))
	{
		// Keep the two-primary rule so zombies scripts and the HUD stay happy.
		primaries = self getWeaponsListPrimaries();
		if (primaries.size >= 2)
			self takeWeapon(self getCurrentWeapon());
	}

	self giveWeapon(weapon);
	self giveMaxAmmo(weapon);
	if (!self is_offhand(weapon))
		self switchToWeapon(weapon);
	self iprintln("Given ^3" + weapon);
}

give_random_weapon()
{
	pool = self weapon_pool();
	if (pool.size == 0)
		return;
	self give_weapon(pool[randomInt(pool.size)]);
}

refill_ammo()
{
	weapons = self getWeaponsList();
	for (i = 0; i < weapons.size; i++)
	{
		self giveMaxAmmo(weapons[i]);
		self setWeaponAmmoClip(weapons[i], weaponClipSize(weapons[i]));
	}
	self iprintln("Ammo refilled");
}

take_current_weapon()
{
	weapon = self getCurrentWeapon();
	if (weapon == "none")
		return;
	self takeWeapon(weapon);
	primaries = self getWeaponsListPrimaries();
	if (primaries.size > 0)
		self switchToWeapon(primaries[0]);
	self iprintln("Took ^3" + weapon);
}

// Der Riese registers upgraded weapons as "<name>_upgraded".
pack_a_punch()
{
	weapon = self getCurrentWeapon();
	upgraded = weapon + "_upgraded";
	if (!isDefined(level.zombie_weapons) || !isDefined(level.zombie_weapons[upgraded]))
	{
		self iprintln("^1No Pack-a-Punch version of " + weapon + " on this map");
		return;
	}
	self takeWeapon(weapon);
	self giveWeapon(upgraded);
	self giveMaxAmmo(upgraded);
	self switchToWeapon(upgraded);
	self iprintlnbold("^5PACK-A-PUNCHED!");
}

rapid_fire_set(on)
{
	if (on)
	{
		self setPerk("specialty_rof");
		mm_set_dvar("perk_weapRateMultiplier", "0.25");
	}
	else
	{
		self unsetPerk("specialty_rof");
		mm_restore_dvar("perk_weapRateMultiplier", "0.75");
	}
	self mm_onoff("Rapid Fire", on);
}

fast_reload_set(on)
{
	if (on)
	{
		self setPerk("specialty_fastreload");
		mm_set_dvar("perk_weapReloadMultiplier", "0.1");
	}
	else
	{
		self unsetPerk("specialty_fastreload");
		mm_restore_dvar("perk_weapReloadMultiplier", "0.5");
	}
	self mm_onoff("Fast Reload", on);
}

// ---------------------------------------------------------------------------
// Bullet modes
// ---------------------------------------------------------------------------

bullet_mode_names()
{
	names = [];
	names[names.size] = "Normal";
	names[names.size] = "Explosive";
	names[names.size] = "Teleport";
	names[names.size] = "Gib Blaster";
	names[names.size] = "Ragdoll Cannon";
	names[names.size] = "Tesla Chain";
	names[names.size] = "Magic Missile";
	names[names.size] = "Airstrike";
	names[names.size] = "Black Hole";
	names[names.size] = "Portal Gun";
	names[names.size] = "Prop Cannon";
	names[names.size] = "AI Summoner";
	names[names.size] = "FX Gun";
	return names;
}

bullet_mode_set(index)
{
	names = bullet_mode_names();
	self.mm_bullet_mode = names[index];
	self notify("mm_stop_bullets");
	self.mm_portals = undefined;
	self.mm_portal_next = undefined;
	self.mm_portal_thread = undefined;
	self iprintln("Bullets: ^3" + self.mm_bullet_mode);

	if (self.mm_bullet_mode == "Normal")
		return;

	if (self.mm_bullet_mode == "Magic Missile")
	{
		self.mm_missile_weapon = self find_projectile_weapon();
		if (!isDefined(self.mm_missile_weapon))
			self iprintln("^1No rocket weapon loaded on this map - using explosions instead");
	}

	self thread bullet_watch();
}

find_projectile_weapon()
{
	pool = self weapon_pool();
	for (i = 0; i < pool.size; i++)
	{
		if (weaponType(pool[i]) == "projectile")
			return pool[i];
	}
	return undefined;
}

bullet_watch()
{
	self endon("disconnect");
	self endon("mm_stop_bullets");
	for (;;)
	{
		self waittill("weapon_fired");
		trace = self mm_trace();
		self thread bullet_effect(self.mm_bullet_mode, trace);
	}
}

bullet_target(trace, radius)
{
	if (isDefined(trace["entity"]) && isAI(trace["entity"]) && isAlive(trace["entity"]))
		return trace["entity"];
	near = mm_ai_near(trace["position"], radius);
	for (i = 0; i < near.size; i++)
	{
		if (near[i].team != "allies")
			return near[i];
	}
	return undefined;
}

bullet_effect(mode, trace)
{
	self endon("disconnect");
	pos = trace["position"];
	dir = self mm_forward();

	switch (mode)
	{
	case "Explosive":
		mm_explode(pos, 200, self);
		break;
	case "Teleport":
		self setOrigin(pos + trace["normal"] * 24);
		break;
	case "Gib Blaster":
		target = bullet_target(trace, 72);
		if (isDefined(target))
		{
			mm_gib(target, mm_random_gib_ref());
			mm_blood(target.origin + (0, 0, 40));
			wait 0.05;
			mm_kill(target, self);
		}
		break;
	case "Ragdoll Cannon":
		target = bullet_target(trace, 72);
		if (isDefined(target))
			thread mm_fling(target, dir * 450 + (0, 0, 350), self);
		break;
	case "Tesla Chain":
		target = bullet_target(trace, 72);
		if (isDefined(target))
			self thread tesla_chain(target, 6);
		break;
	case "Magic Missile":
		if (isDefined(self.mm_missile_weapon))
			magicBullet(self.mm_missile_weapon, self getEye() + dir * 40, pos, self);
		else
			mm_explode(pos, 200, self);
		break;
	case "Airstrike":
		self thread airstrike(pos);
		break;
	case "Black Hole":
		if (!isDefined(self.mm_black_hole) || getTime() > self.mm_black_hole)
			self thread black_hole(pos);
		break;
	case "Portal Gun":
		self thread portal_place(pos + trace["normal"] * 32);
		break;
	case "Prop Cannon":
		self thread prop_cannon(pos, dir);
		break;
	case "AI Summoner":
		self thread summon_at(pos + trace["normal"] * 16);
		break;
	case "FX Gun":
		if (isDefined(self.mm_fx_key) && isDefined(level._effect[self.mm_fx_key]))
			playFX(level._effect[self.mm_fx_key], pos);
		else
			self iprintln("^1Pick an effect in Fun > FX Browser first");
		break;
	default:
		break;
	}
}

tesla_chain(first, jumps)
{
	current = first;
	hit = [];
	for (i = 0; i < jumps && isDefined(current); i++)
	{
		hit[hit.size] = current;
		from = current.origin; // the corpse may be gone by the time we look for the next target
		current setElectrified(1.5);
		mm_blood(from + (0, 0, 50));
		thread mm_fling(current, (randomIntRange(-100, 100), randomIntRange(-100, 100), 250), self);
		wait 0.15;

		next = undefined;
		best = 0;
		enemies = mm_enemies();
		for (j = 0; j < enemies.size; j++)
		{
			if (!isAlive(enemies[j]) || mm_array_contains(hit, enemies[j]))
				continue;
			d = distance(from, enemies[j].origin);
			if (d < 400 && (!isDefined(next) || d < best))
			{
				next = enemies[j];
				best = d;
			}
		}
		current = next;
	}
}

airstrike(pos)
{
	for (i = 0; i < 6; i++)
	{
		mm_explode(mm_ground(pos + (randomFloatRange(-220, 220), randomFloatRange(-220, 220), 64)), 220, self);
		wait 0.2;
	}
}

black_hole(pos)
{
	self endon("disconnect");
	self.mm_black_hole = getTime() + 7000; // one at a time; expires even if this thread dies
	self iprintln("^5Black hole opened");
	core = pos + (0, 0, 48);
	fx = mm_fx("thunder");

	for (t = 0; t < 60; t++)
	{
		physicsExplosionSphere(core, 600, 100, -2);
		ai = mm_ai_near(core, 700);
		for (i = 0; i < ai.size; i++)
		{
			if (ai[i].team == "allies")
				continue;
			if (distance(ai[i].origin, core) < 90)
			{
				mm_blood(ai[i].origin + (0, 0, 40));
				thread mm_fling(ai[i], (0, 0, 600), self);
			}
			else
			{
				pull = vectorNormalize(core - ai[i].origin) * 40;
				ai[i] teleport(ai[i].origin + mm_flat(pull));
			}
		}
		if (t % 10 == 0)
		{
			mm_play_fx(fx, core);
			earthquake(0.2, 0.5, core, 900);
		}
		wait 0.1;
	}
	mm_explode(core, 350, self);
	self.mm_black_hole = undefined;
}

// Alternates between placing the orange and blue portal. Players and AI stepping into one
// come out of the other.
portal_place(pos)
{
	if (!isDefined(self.mm_portals))
		self.mm_portals = [];

	index = 0;
	if (isDefined(self.mm_portal_next))
		index = self.mm_portal_next;
	self.mm_portal_next = 1 - index;

	self.mm_portals[index] = pos;
	mm_blood(pos);
	if (index == 0)
		self iprintln("^3Orange ^7portal placed");
	else
		self iprintln("^5Blue ^7portal placed");

	if (self.mm_portals.size == 2 && !isDefined(self.mm_portal_thread))
		self thread portal_think();
}

portal_think()
{
	self endon("disconnect");
	self endon("mm_stop_bullets");
	self.mm_portal_thread = true;
	fx = mm_fx("thunder");
	cooldown = [];

	for (tick = 0;; tick++)
	{
		for (p = 0; p < 2; p++)
		{
			src = self.mm_portals[p];
			dst = self.mm_portals[1 - p];
			if (tick % 20 == 0)
				mm_play_fx(fx, src);

			players = get_players();
			for (i = 0; i < players.size; i++)
			{
				key = "p" + players[i] getEntityNumber();
				if (isDefined(cooldown[key]) && cooldown[key] > getTime())
					continue;
				if (distance(players[i].origin, src) < 48)
				{
					players[i] setOrigin(dst);
					cooldown[key] = getTime() + 1500;
				}
			}

			ai = mm_ai_near(src, 48);
			for (i = 0; i < ai.size; i++)
			{
				key = "a" + ai[i] getEntityNumber();
				if (isDefined(cooldown[key]) && cooldown[key] > getTime())
					continue;
				ai[i] teleport(dst);
				cooldown[key] = getTime() + 1500;
			}
		}
		wait 0.05;
	}
}

prop_cannon(pos, dir)
{
	models = maps\mod_menu\forge::model_list();
	if (models.size == 0)
	{
		mm_explode(pos, 150, self);
		return;
	}
	start = self getEye() + dir * 60;
	prop = spawn("script_model", start);
	prop setModel(models[randomInt(models.size)]);
	prop.angles = (randomInt(360), randomInt(360), 0);
	prop moveGravity(dir * 1400 + (0, 0, 120), 6);
	wait 6;
	if (isDefined(prop))
		prop delete();
}

summon_at(pos)
{
	if (isDefined(self.mm_summon_cooldown) && self.mm_summon_cooldown > getTime())
		return;
	self.mm_summon_cooldown = getTime() + 600;
	maps\mod_menu\ai::spawn_random_enemy_at(pos);
}

// ---------------------------------------------------------------------------
// Aim toys
// ---------------------------------------------------------------------------

aimbot_set(on)
{
	self notify("mm_stop_aimbot");
	self mm_onoff("Aimbot (hold LT)", on);
	if (!on)
		return;

	self endon("mm_stop_aimbot");
	self endon("disconnect");
	for (;;)
	{
		wait 0.05;
		if (!isAlive(self) || self.mm_open || !self adsButtonPressed())
			continue;
		target = self best_visible_enemy(40);
		if (isDefined(target))
			self setPlayerAngles(vectorToAngles(target getEye() - self getEye()));
	}
}

// Closest living enemy within maxAngle degrees of the view that can be seen.
best_visible_enemy(maxAngle)
{
	eye = self getEye();
	forward = self mm_forward();
	minDot = cos(maxAngle);
	best = undefined;
	bestDot = 0;
	enemies = mm_enemies();
	for (i = 0; i < enemies.size; i++)
	{
		if (!isAlive(enemies[i]))
			continue;
		head = enemies[i] getEye();
		dot = vectorDot(forward, vectorNormalize(head - eye));
		if (dot < minDot || dot < bestDot)
			continue;
		if (!sightTracePassed(eye, head, false, self))
			continue;
		best = enemies[i];
		bestDot = dot;
	}
	return best;
}

death_stare_set(on)
{
	self notify("mm_stop_stare");
	self mm_onoff("Death Stare", on);
	if (!on)
		return;

	self endon("mm_stop_stare");
	self endon("disconnect");
	for (;;)
	{
		wait 0.1;
		if (!isAlive(self) || self.mm_open)
			continue;
		target = self best_visible_enemy(6);
		if (isDefined(target))
		{
			mm_blood(target getEye());
			thread mm_fling(target, self mm_forward() * 300 + (0, 0, 450), self);
		}
	}
}
