/*
	Nazi Zombies features.

	Campaign levels do not contain the _zombiemode scripts, and referencing a missing script is a
	fatal compile error, so nothing here calls into them. Instead it drives the same level data
	and entity triggers the zombies scripts listen to, which also keeps it working across
	Nacht der Untoten, Verrueckt, Shi No Numa, Der Riese and custom maps.
*/

#include maps\_utility;
#include common_scripts\utility;
#include maps\mod_menu\util;

// ---------------------------------------------------------------------------
// Points
// ---------------------------------------------------------------------------

add_points(amount)
{
	self.score += amount;
	if (isDefined(self.score_total))
		self.score_total += amount;
	self iprintln("^2+" + amount + " ^7points");
}

reset_points()
{
	self.score = 500;
	self iprintln("Points reset to 500");
}

infinite_points_set(on)
{
	self notify("mm_stop_points");
	self mm_onoff("Infinite Points", on);
	if (!on)
		return;

	self endon("mm_stop_points");
	self endon("disconnect");
	for (;;)
	{
		if (self.score < 900000)
			self.score = 1000000;
		wait 0.25;
	}
}

// ---------------------------------------------------------------------------
// Perks
// ---------------------------------------------------------------------------

perk_list()
{
	perks = [];
	perks[perks.size] = "specialty_armorvest";
	perks[perks.size] = "specialty_quickrevive";
	perks[perks.size] = "specialty_fastreload";
	perks[perks.size] = "specialty_rof";
	return perks;
}

perk_shader(perk)
{
	switch (perk)
	{
	case "specialty_armorvest":
		return "specialty_juggernaut_zombies";
	case "specialty_quickrevive":
		return "specialty_quickrevive_zombies";
	case "specialty_fastreload":
		return "specialty_fastreload_zombies";
	case "specialty_rof":
		return "specialty_doubletap_zombies";
	default:
		return "";
	}
}

give_perk(perk)
{
	if (self hasPerk(perk))
	{
		self iprintln("You already have " + perk);
		return;
	}

	self setPerk(perk);
	if (perk == "specialty_armorvest")
	{
		self.maxhealth = 160;
		self.health = 160;
	}
	self setBlur(4, 0.1);
	wait 0.1;
	self setBlur(0, 0.1);

	// Same HUD slot layout as maps\_zombiemode_perks::perk_hud_create(). Separate thread so a
	// missing perk icon (Nacht has no perk machines) cannot stop the perk itself.
	self thread perk_icon(perk);
	self thread perk_lose_on_down(perk);
	self iprintln("Perk: ^3" + perk);
}

perk_icon(perk)
{
	if (!isDefined(self.perk_hud))
		self.perk_hud = [];
	if (isDefined(self.perk_hud[perk]))
		return;

	hud = newClientHudElem(self);
	hud.foreground = true;
	hud.sort = 1;
	hud.hidewheninmenu = false;
	hud.alignX = "left";
	hud.alignY = "bottom";
	hud.horzAlign = "left";
	hud.vertAlign = "bottom";
	hud.x = self.perk_hud.size * 30;
	hud.y = -70;
	hud.alpha = 1;
	self.perk_hud[perk] = hud;
	hud setShader(perk_shader(perk), 24, 24);
}

perk_lose_on_down(perk)
{
	self endon("disconnect");
	self waittill_any("fake_death", "death", "player_downed");
	self unsetPerk(perk);
	if (perk == "specialty_armorvest")
		self.maxhealth = 100;
	if (isDefined(self.perk_hud) && isDefined(self.perk_hud[perk]))
	{
		self.perk_hud[perk] destroy();
		self.perk_hud[perk] = undefined;
	}
}

give_all_perks()
{
	perks = perk_list();
	for (i = 0; i < perks.size; i++)
	{
		if (!self hasPerk(perks[i]))
			self thread give_perk(perks[i]);
	}
}

// ---------------------------------------------------------------------------
// Doors & power
// ---------------------------------------------------------------------------

// door_think()/debris_think() only open when the buyer is holding Use, so wait for X to be held,
// cover the cost, fire the trigger and give the points back.
open_all_doors()
{
	self endon("disconnect");

	// X is also the menu's select button, so pressing it here can start a second copy.
	if (isDefined(self.mm_opening_doors) && self.mm_opening_doors > getTime())
		return;
	self.mm_opening_doors = getTime() + 30000;

	triggers = getEntArray("zombie_door", "targetname");
	debris = getEntArray("zombie_debris", "targetname");
	for (i = 0; i < debris.size; i++)
		triggers[triggers.size] = debris[i];

	if (triggers.size == 0)
	{
		self.mm_opening_doors = undefined;
		self iprintln("^1No doors on this map");
		return;
	}

	if (!self useButtonPressed())
	{
		self iprintlnbold("Hold ^3[X]^7 to open every door");
		for (t = 0; t < 100 && !self useButtonPressed(); t++)
			wait 0.05;
		if (!self useButtonPressed())
		{
			self.mm_opening_doors = undefined;
			return;
		}
	}

	startScore = self.score;
	opened = 0;
	for (i = 0; i < triggers.size && self useButtonPressed(); i++)
	{
		if (!isDefined(triggers[i]))
			continue;
		cost = 0;
		if (isDefined(triggers[i].zombie_cost))
			cost = triggers[i].zombie_cost;
		self.score = startScore + cost + 10;
		triggers[i] notify("trigger", self);
		opened++;
		wait 0.1;
	}
	self.score = startScore;
	self.mm_opening_doors = undefined;
	self iprintln("Opened ^3" + opened + "^7 doors and debris piles (free)");
}

power_on()
{
	names = [];
	names[names.size] = "use_power_switch";
	names[names.size] = "use_master_switch";
	names[names.size] = "use_elec_switch";
	for (i = 0; i < names.size; i++)
	{
		trig = getEnt(names[i], "targetname");
		if (isDefined(trig))
		{
			trig notify("trigger", self);
			self iprintlnbold("^3Power is ON");
			return;
		}
	}
	self iprintln("^1This map has no power switch");
}

// ---------------------------------------------------------------------------
// Power-ups
// ---------------------------------------------------------------------------

powerup_names()
{
	if (isDefined(level.zombie_powerup_array))
		return level.zombie_powerup_array;
	return [];
}

// Make maps\_zombiemode_powerups::powerup_drop() pick this power-up for the next drop.
rig_drop(name)
{
	list = powerup_names();
	for (i = 0; i < list.size; i++)
	{
		if (list[i] == name)
		{
			level.zombie_powerup_index = i;
			level.zombie_vars["zombie_drop_item"] = 1;
			level.powerup_drop_count = 0;
			return true;
		}
	}
	return false;
}

next_drop(name)
{
	if (!rig_drop(name))
		return;
	self iprintln("Next zombie killed drops ^3" + name);
}

// Kill the closest zombie right in front of you so the drop lands at your feet.
drop_now(name)
{
	self endon("disconnect");
	if (!rig_drop(name))
		return;

	zombie = mm_closest(self.origin, mm_zombies());
	if (!isDefined(zombie))
	{
		self iprintln("^1Needs a live zombie - it will drop on the next kill instead");
		return;
	}
	spot = mm_ground(self.origin + mm_flat(self mm_forward()) * 90 + (0, 0, 32));
	zombie teleport(spot);
	wait 0.1;
	mm_kill(zombie, self);
	self iprintln("Dropped ^3" + name);
}

// ---------------------------------------------------------------------------
// Rounds
// ---------------------------------------------------------------------------

skip_round()
{
	level.zombie_total = 0;
	enemies = mm_enemies();
	for (i = 0; i < enemies.size; i++)
		mm_kill(enemies[i]);
	mm_announce("^1Round skipped");
}

jump_to_round(round)
{
	if (!isDefined(level.round_number))
		return;
	level.round_number = int(round) - 1;
	skip_round();
	mm_announce("Next round: ^1" + int(round));
}

// ---------------------------------------------------------------------------
// Zombie behaviour
// ---------------------------------------------------------------------------

speed_names()
{
	names = [];
	names[names.size] = "Default";
	names[names.size] = "Walkers";
	names[names.size] = "Runners";
	names[names.size] = "Sprinters";
	return names;
}

speed_set(index)
{
	level notify("mm_stop_zombie_speed");
	names = speed_names();
	self iprintln("Zombie speed: ^3" + names[index]);
	if (index == 0)
		return;

	prefixes = [];
	prefixes[1] = "walk";
	prefixes[2] = "run";
	prefixes[3] = "sprint";
	counts = [];
	counts[1] = 8;
	counts[2] = 6;
	counts[3] = 4;

	level endon("mm_stop_zombie_speed");
	for (;;)
	{
		zombies = mm_zombies();
		for (i = 0; i < zombies.size; i++)
		{
			z = zombies[i];
			if (!isAlive(z) || (isDefined(z.has_legs) && !z.has_legs) || (isDefined(z.mm_speed) && z.mm_speed == index))
				continue;
			z.mm_speed = index;
			z.zombie_move_speed = prefixes[index];
			z set_zombie_anim(prefixes[index] + randomIntRange(1, counts[index] + 1));
		}
		wait 0.5;
	}
}

set_zombie_anim(key)
{
	if (!isDefined(level.scr_anim) || !isDefined(level.scr_anim["zombie"]) || !isDefined(level.scr_anim["zombie"][key]))
		return;
	self set_run_anim(key);
	self.run_combatanim = level.scr_anim["zombie"][key];
}

// Blow the legs off every zombie. Mirrors maps\_zombiemode_spawner::zombie_gib_on_damage().
crawlers()
{
	if (!isDefined(level.scr_anim) || !isDefined(level.scr_anim["zombie"]))
		return;
	zombies = mm_zombies();
	count = 0;
	keys = [];
	keys[keys.size] = "crawl1";
	keys[keys.size] = "crawl2";
	keys[keys.size] = "crawl3";
	keys[keys.size] = "crawl4";
	keys[keys.size] = "crawl5";
	keys[keys.size] = "crawl_hand_1";
	for (i = 0; i < zombies.size; i++)
	{
		z = zombies[i];
		if (!isAlive(z) || (isDefined(z.has_legs) && !z.has_legs))
			continue;
		key = keys[randomInt(keys.size)];
		if (!isDefined(level.scr_anim["zombie"][key]))
			continue;
		mm_gib(z, "no_legs");
		z.has_legs = false;
		z allowedStances("crouch");
		z set_run_anim(key);
		z.run_combatanim = level.scr_anim["zombie"][key];
		z.crouchRunAnim = level.scr_anim["zombie"][key];
		z.crouchrun_combatanim = level.scr_anim["zombie"][key];
		count++;
	}
	mm_announce("^1" + count + " zombies lost their legs");
}

headless()
{
	zombies = mm_zombies();
	for (i = 0; i < zombies.size; i++)
	{
		if (!isAlive(zombies[i]))
			continue;
		mm_gib(zombies[i], "head");
		mm_blood(zombies[i].origin + (0, 0, 64));
	}
	mm_announce("^1The headless horde");
}

// ---------------------------------------------------------------------------
// Co-op
// ---------------------------------------------------------------------------

revive_all()
{
	players = get_players();
	count = 0;
	for (i = 0; i < players.size; i++)
	{
		if (players[i] maps\_laststand::player_is_in_laststand())
		{
			players[i] maps\_laststand::revive_force_revive(self);
			count++;
		}
	}
	self iprintln("Revived ^3" + count + "^7 players");
}

zombie_counter_set(on)
{
	self notify("mm_stop_counter");
	if (isDefined(self.mm_counter))
	{
		self.mm_counter destroy();
		self.mm_counter_label destroy();
		self.mm_counter = undefined;
	}
	self mm_onoff("Zombie Counter", on);
	if (!on)
		return;

	self endon("mm_stop_counter");
	self endon("disconnect");

	label = newClientHudElem(self);
	label.alignX = "right";
	label.alignY = "top";
	label.horzAlign = "right";
	label.vertAlign = "top";
	label.x = -46;
	label.y = 8;
	label.font = "default";
	label.fontScale = 1.4;
	label.foreground = true;
	label.hidewheninmenu = true;
	label setText("Zombies left:");
	self.mm_counter_label = label;

	counter = newClientHudElem(self);
	counter.alignX = "right";
	counter.alignY = "top";
	counter.horzAlign = "right";
	counter.vertAlign = "top";
	counter.x = -8;
	counter.y = 8;
	counter.font = "default";
	counter.fontScale = 1.4;
	counter.color = (1, 0.3, 0.3);
	counter.foreground = true;
	counter.hidewheninmenu = true;
	self.mm_counter = counter;

	for (;;)
	{
		remaining = mm_enemies().size;
		if (isDefined(level.zombie_total))
			remaining += level.zombie_total;
		counter setValue(remaining);
		wait 0.5;
	}
}
