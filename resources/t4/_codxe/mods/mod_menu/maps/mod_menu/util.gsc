/*
	Shared helpers for the mod menu.

	Everything here must work on every singleplayer map (campaign and zombies), so it only
	touches builtins and scripts that _load.gsc pulls into every level.
*/

#include maps\_utility;
#include common_scripts\utility;

// ---------------------------------------------------------------------------
// Messages
// ---------------------------------------------------------------------------

mm_msg(text)
{
	self iprintln(text);
}

mm_bold(text)
{
	self iprintlnbold(text);
}

mm_announce(text)
{
	players = get_players();
	for (i = 0; i < players.size; i++)
		players[i] iprintlnbold(text);
}

mm_onoff(label, on)
{
	if (on)
		self iprintln(label + " [^2ON^7]");
	else
		self iprintln(label + " [^1OFF^7]");
}

// ---------------------------------------------------------------------------
// Menu state (written by core.gsc)
// ---------------------------------------------------------------------------

mm_get(key, scope, def)
{
	if (scope == "level")
		value = level.mm_state[key];
	else
		value = self.mm_state[key];
	if (!isDefined(value))
		return def;
	return value;
}

mm_level_vision()
{
	if (isDefined(level.lvl_visionset))
		return level.lvl_visionset;
	return level.script;
}

// ---------------------------------------------------------------------------
// Math
// ---------------------------------------------------------------------------

mm_abs(value)
{
	if (value < 0)
		return value * -1;
	return value;
}

mm_clamp(value, low, high)
{
	if (value < low)
		return low;
	if (value > high)
		return high;
	return value;
}

mm_flat(vec)
{
	return (vec[0], vec[1], 0);
}

mm_random_vec(radius)
{
	return (randomFloatRange(radius * -1, radius), randomFloatRange(radius * -1, radius), randomFloatRange(radius * -1, radius));
}

// ---------------------------------------------------------------------------
// Traces
// ---------------------------------------------------------------------------

mm_forward()
{
	return anglesToForward(self getPlayerAngles());
}

// bulletTrace from the eye along the view direction.
mm_trace()
{
	eye = self getEye();
	return bulletTrace(eye, eye + self mm_forward() * 20000, true, self);
}

// Point just in front of whatever the player is aiming at.
mm_aim_pos()
{
	trace = self mm_trace();
	return trace["position"] - self mm_forward() * 12;
}

mm_ground(pos)
{
	trace = bulletTrace(pos + (0, 0, 16), pos - (0, 0, 20000), false, undefined);
	return trace["position"];
}

mm_ceiling(pos, maxHeight)
{
	trace = bulletTrace(pos + (0, 0, 16), pos + (0, 0, maxHeight), false, undefined);
	return trace["position"];
}

// ---------------------------------------------------------------------------
// Mode checks
// ---------------------------------------------------------------------------

mm_is_zombies()
{
	return isDefined(level.zombiemode) && level.zombiemode;
}

mm_is_host()
{
	players = get_players();
	return players.size > 0 && players[0] == self;
}

// ---------------------------------------------------------------------------
// Dvars
// ---------------------------------------------------------------------------

// Some dvars are "saved" (SetSavedDvar only) and some are not (SetDvar only). Calling the wrong
// one is a runtime script error, which only kills the calling thread, so try both in isolation.
mm_set_dvar(name, value)
{
	mm_remember_dvar(name);
	level thread mm_try_set_saved_dvar(name, value);
	level thread mm_try_set_dvar(name, value);
}

mm_try_set_saved_dvar(name, value)
{
	setSavedDvar(name, value);
}

mm_try_set_dvar(name, value)
{
	setDvar(name, value);
}

mm_remember_dvar(name)
{
	if (!isDefined(level.mm_dvar_defaults))
		level.mm_dvar_defaults = [];
	if (!isDefined(level.mm_dvar_defaults[name]))
		level.mm_dvar_defaults[name] = getDvar(name);
}

mm_restore_dvar(name, fallback)
{
	value = fallback;
	if (isDefined(level.mm_dvar_defaults) && isDefined(level.mm_dvar_defaults[name]) && level.mm_dvar_defaults[name] != "")
		value = level.mm_dvar_defaults[name];
	mm_set_dvar(name, value);
}

// ---------------------------------------------------------------------------
// AI
// ---------------------------------------------------------------------------

mm_enemies()
{
	return GetAISpeciesArray("axis", "all");
}

mm_all_ai()
{
	return getAIArray();
}

mm_zombies()
{
	result = [];
	enemies = GetAISpeciesArray("axis", "all");
	for (i = 0; i < enemies.size; i++)
	{
		if (isDefined(enemies[i].animname) && enemies[i].animname == "zombie")
			result[result.size] = enemies[i];
	}
	return result;
}

mm_closest(origin, ents)
{
	best = undefined;
	bestDist = 0;
	for (i = 0; i < ents.size; i++)
	{
		if (!isDefined(ents[i]))
			continue;
		d = distanceSquared(origin, ents[i].origin);
		if (!isDefined(best) || d < bestDist)
		{
			best = ents[i];
			bestDist = d;
		}
	}
	return best;
}

mm_ai_near(origin, radius)
{
	result = [];
	ai = getAIArray();
	for (i = 0; i < ai.size; i++)
	{
		if (isAlive(ai[i]) && distance(origin, ai[i].origin) <= radius)
			result[result.size] = ai[i];
	}
	return result;
}

mm_kill(ai, attacker)
{
	if (!isDefined(ai) || !isAlive(ai))
		return;
	if (isDefined(attacker))
		ai doDamage(ai.health + 1000, ai.origin, attacker);
	else
		ai doDamage(ai.health + 1000, ai.origin);
}

// Kill an AI and throw its ragdoll. Runs threaded so a ragdoll failure cannot stop the caller.
mm_fling(ai, velocity, attacker)
{
	if (!isDefined(ai) || !isAlive(ai))
		return;
	origin = ai.origin;
	mm_kill(ai, attacker);
	wait 0.05;
	if (isDefined(ai))
	{
		ai startRagdoll();
		ai launchRagdoll(velocity);
	}
	physicsExplosionSphere(origin - vectorNormalize(velocity) * 24, 96, 32, 2);
}

mm_gib(ai, gibRef)
{
	if (!isDefined(ai) || !isAlive(ai))
		return;
	if (!isDefined(ai.a))
		return;
	ai.a.gib_ref = gibRef;
	ai thread animscripts\death::do_gib();
}

mm_random_gib_ref()
{
	refs = [];
	refs[refs.size] = "head";
	refs[refs.size] = "right_arm";
	refs[refs.size] = "left_arm";
	refs[refs.size] = "right_leg";
	refs[refs.size] = "left_leg";
	refs[refs.size] = "no_legs";
	refs[refs.size] = "guts";
	return refs[randomInt(refs.size)];
}

// ---------------------------------------------------------------------------
// Effects
// ---------------------------------------------------------------------------

mm_fx(name)
{
	if (isDefined(level._effect) && isDefined(level._effect[name]))
		return level._effect[name];
	return undefined;
}

mm_play_fx(fx, pos)
{
	if (isDefined(fx))
		playFX(fx, pos);
}

mm_blood(pos)
{
	if (isDefined(anim._effect) && isDefined(anim._effect["animscript_gib_fx"]))
		playFX(anim._effect["animscript_gib_fx"], pos);
}

mm_sound(pos, alias)
{
	if (soundExists(alias))
		playSoundAtPosition(alias, pos);
}

// Explosion that never hurts players. _collectibles.gsc loads "thunder" (the default explosion)
// on every singleplayer map, so that is the visual. Stock SP scripts only ever call
// RadiusDamage with four arguments, so the attacker is not passed to it.
mm_explode(pos, radius, attacker)
{
	mm_play_fx(mm_fx("thunder"), pos);
	mm_sound(pos, "grenade_explode_default");
	setPlayerIgnoreRadiusDamage(true);
	radiusDamage(pos, radius, 600, 100);
	setPlayerIgnoreRadiusDamage(false);
	physicsExplosionSphere(pos, radius, radius * 0.25, 1.5);
	earthquake(0.4, 0.75, pos, radius * 4);
}

mm_flash(color, duration)
{
	flash = newClientHudElem(self);
	flash.x = 0;
	flash.y = 0;
	flash.alignX = "left";
	flash.alignY = "top";
	flash.horzAlign = "fullscreen";
	flash.vertAlign = "fullscreen";
	flash.foreground = true;
	flash.sort = 1000;
	flash.color = color;
	flash.alpha = 0.85;
	flash setShader("white", 640, 480);
	flash fadeOverTime(duration);
	flash.alpha = 0;
	wait duration;
	flash destroy();
}

// ---------------------------------------------------------------------------
// Entities
// ---------------------------------------------------------------------------

// tag_origin is precached by _load.gsc on every map; it gives a tagged, invisible mover.
mm_spawn_mover(origin, angles)
{
	mover = spawn("script_model", origin);
	mover setModel("tag_origin");
	if (isDefined(angles))
		mover.angles = angles;
	return mover;
}

mm_array_contains(array, value)
{
	for (i = 0; i < array.size; i++)
	{
		if (array[i] == value)
			return true;
	}
	return false;
}

mm_array_add_unique(array, value)
{
	if (!isDefined(value) || value == "" || value == "none")
		return array;
	if (!mm_array_contains(array, value))
		array[array.size] = value;
	return array;
}

// Menu labels share one 255 character HUD string per page, so keep them short.
mm_short(text, maxLength)
{
	if (text.size <= maxLength)
		return text;
	return getSubStr(text, text.size - maxLength, text.size);
}
