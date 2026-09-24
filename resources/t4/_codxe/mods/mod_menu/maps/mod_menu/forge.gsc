/*
	Forge: spawn and move props. The model list is discovered at runtime from script_models the
	map already placed plus weapon world models, so every entry is guaranteed to be loaded.
	"Solid" spawns use SpawnCollision(), a GSC function CoD Xe adds to T4 singleplayer.
*/

#include maps\_utility;
#include common_scripts\utility;
#include maps\mod_menu\util;

model_list()
{
	if (isDefined(level.mm_models))
		return level.mm_models;

	models = [];
	ents = getEntArray("script_model", "classname");
	for (i = 0; i < ents.size && models.size < 50; i++)
	{
		if (isDefined(ents[i].model) && ents[i].model != "tag_origin" && ents[i].model != "")
			models = mm_array_add_unique(models, ents[i].model);
	}

	weapons = maps\mod_menu\weapons::weapon_pool();
	for (i = 0; i < weapons.size && models.size < 60; i++)
		models = mm_array_add_unique(models, getWeaponModel(weapons[i]));

	level.mm_models = models;
	return models;
}

forge_track(ent)
{
	if (!isDefined(self.mm_forge))
		self.mm_forge = [];
	self.mm_forge[self.mm_forge.size] = ent;
}

spawn_model(model)
{
	trace = self mm_trace();
	pos = trace["position"];
	view = self getPlayerAngles();
	angles = (0, view[1], 0);

	if (self mm_get("forge_solid", "self", false))
		ent = spawnCollision(model, "mm_forge", pos, angles);
	else
	{
		ent = spawn("script_model", pos);
		ent setModel(model);
		ent.angles = angles;
	}

	self forge_track(ent);
	self.mm_forge_model = model;

	if (self mm_get("forge_physics", "self", false) && modelHasPhysPreset(model))
		ent physicsLaunch(ent.origin, (0, 0, -10));

	self iprintln("Spawned ^3" + model);
}

solid_set(on)
{
	self mm_onoff("Solid Spawns (CoD Xe SpawnCollision)", on);
}

physics_set(on)
{
	self mm_onoff("Physics Spawns", on);
}

undo_spawn()
{
	if (!isDefined(self.mm_forge) || self.mm_forge.size == 0)
	{
		self iprintln("Nothing to undo");
		return;
	}
	last = self.mm_forge[self.mm_forge.size - 1];
	if (isDefined(last))
		last delete();
	self.mm_forge[self.mm_forge.size - 1] = undefined;
	self iprintln("Removed last spawn");
}

clear_spawns()
{
	if (!isDefined(self.mm_forge))
		return;
	count = 0;
	for (i = 0; i < self.mm_forge.size; i++)
	{
		if (isDefined(self.mm_forge[i]))
		{
			self.mm_forge[i] delete();
			count++;
		}
	}
	self.mm_forge = [];
	self iprintln("Deleted ^3" + count + "^7 props");
}

aimed_entity()
{
	trace = self mm_trace();
	ent = trace["entity"];
	if (!isDefined(ent) || isPlayer(ent))
		return undefined;
	return ent;
}

delete_aimed()
{
	ent = self aimed_entity();
	if (!isDefined(ent))
	{
		self iprintln("^1Aim at a prop, door or AI");
		return;
	}
	if (isAI(ent))
		mm_kill(ent, self);
	else
		ent delete();
	self iprintln("Deleted");
}

launch_aimed()
{
	ent = self aimed_entity();
	if (!isDefined(ent))
	{
		self iprintln("^1Aim at something first");
		return;
	}
	if (isAI(ent))
	{
		thread mm_fling(ent, self mm_forward() * 900 + (0, 0, 400), self);
		return;
	}
	if (isDefined(ent.model) && modelHasPhysPreset(ent.model))
		ent physicsLaunch(ent.origin, self mm_forward() * 60 + (0, 0, 20));
	else
		ent moveGravity(self mm_forward() * 900 + (0, 0, 500), 4);
}

// Hold LB to grab the prop you are aiming at, RB to spin it, release LB to throw it.
pickup_set(on)
{
	self notify("mm_stop_pickup");
	self mm_onoff("Grab Mode (hold LB)", on);
	if (!on)
	{
		self enableOffhandWeapons();
		return;
	}

	self endon("mm_stop_pickup");
	self endon("disconnect");
	for (;;)
	{
		wait 0.05;
		if (!isAlive(self) || self.mm_open)
			continue;
		self disableOffhandWeapons();
		if (!self secondaryOffhandButtonPressed())
			continue;
		ent = self aimed_entity();
		if (isDefined(ent) && !isAI(ent))
			self carry(ent);
	}
}

carry(ent)
{
	dist = distance(self getEye(), ent.origin);
	dist = mm_clamp(dist, 80, 400);
	last = ent.origin;
	while (isDefined(ent) && isAlive(self) && self secondaryOffhandButtonPressed() && !self.mm_open)
	{
		last = ent.origin;
		ent moveTo(self getEye() + self mm_forward() * dist, 0.05);
		if (self fragButtonPressed())
			ent rotateYaw(30, 0.05);
		wait 0.05;
	}
	if (!isDefined(ent))
		return;

	fling = (ent.origin - last) * 20;
	if (isDefined(ent.model) && modelHasPhysPreset(ent.model))
		ent physicsLaunch(ent.origin, fling * 0.05);
}
