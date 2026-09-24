/*
	Fun & chaos toys. These lean on movers (tag_origin script models) and player linking to
	fake physics the stock SP scripting API does not expose directly.
*/

#include maps\_utility;
#include common_scripts\utility;
#include maps\mod_menu\util;

// ---------------------------------------------------------------------------
// Flight
// ---------------------------------------------------------------------------

jetpack_set(on)
{
	self notify("mm_stop_jetpack");
	self mm_onoff("Jetpack (hold A in the air)", on);
	if (!on)
		return;

	self endon("mm_stop_jetpack");
	self endon("disconnect");
	for (;;)
	{
		wait 0.05;
		if (!isAlive(self) || self.mm_open || isDefined(self.mm_riding))
			continue;
		if (self jumpButtonPressed() && !self isOnGround())
			self jetpack_fly();
	}
}

jetpack_fly()
{
	self.mm_riding = true;
	mover = mm_spawn_mover(self.origin, (0, 0, 0));
	self playerLinkToDelta(mover, undefined, 1.0);
	speed = 6;

	while (isAlive(self) && self jumpButtonPressed() && !self.mm_open)
	{
		speed = mm_clamp(speed + 3, 0, 30);
		forward = self mm_forward();
		dir = vectorNormalize((forward[0], forward[1], forward[2] + 0.6));
		dest = mover.origin + dir * speed;
		trace = bulletTrace(mover.origin + (0, 0, 36), dest + (0, 0, 36), false, self);
		if (trace["fraction"] < 1)
			break;
		mover moveTo(dest, 0.05);
		if (randomInt(3) == 0)
			mm_play_fx(mm_fx("thunder"), mover.origin - (0, 0, 16));
		wait 0.05;
	}

	self unlink();
	mover delete();
	self.mm_riding = undefined;
}

// Ride a missile that follows your view. It explodes on whatever it hits.
rocket_ride()
{
	self endon("disconnect");
	if (isDefined(self.mm_riding))
		return;
	self.mm_riding = true;

	wasGod = self.god;
	self.god = true;
	// The mover never rotates: the linked view is relative to it, so turning it with the view
	// would feed back into a spin.
	mover = mm_spawn_mover(self getEye() + self mm_forward() * 64, (0, 0, 0));
	self playerLinkToDelta(mover, undefined, 1.0);
	self iprintlnbold("^1ROCKET RIDE ^7- steer with your view");

	impact = undefined;
	for (t = 0; t < 160 && isAlive(self); t++)
	{
		dir = self mm_forward();
		dest = mover.origin + dir * 70;
		trace = bulletTrace(mover.origin, dest, true, self);
		if (trace["fraction"] < 1)
		{
			impact = trace["position"];
			break;
		}
		mover moveTo(dest, 0.05);
		if (t % 2 == 0)
			mm_play_fx(mm_fx("thunder"), mover.origin - dir * 40);
		wait 0.05;
	}

	if (!isDefined(impact))
		impact = mover.origin;

	self unlink();
	self setOrigin(mm_ground(impact - self mm_forward() * 96 + (0, 0, 32)));
	mover delete();
	mm_explode(impact, 400, self);
	wait 0.5;
	self.god = wasGod;
	self.mm_riding = undefined;
}

// Ballistic launch along the view direction.
human_cannonball()
{
	self endon("disconnect");
	if (isDefined(self.mm_riding))
		return;
	self.mm_riding = true;

	mover = mm_spawn_mover(self.origin + (0, 0, 8), (0, 0, 0));
	self playerLinkToDelta(mover, undefined, 1.0);
	velocity = self mm_forward() * 1400 + (0, 0, 650);
	self iprintlnbold("^3FIRE IN THE HOLE!");

	for (t = 0; t < 100 && isAlive(self); t++)
	{
		velocity = velocity - (0, 0, 800 * 0.05);
		dest = mover.origin + velocity * 0.05;
		trace = bulletTrace(mover.origin + (0, 0, 32), dest + (0, 0, 32), false, self);
		if (trace["fraction"] < 1)
		{
			dest = trace["position"] - (0, 0, 32) - vectorNormalize(velocity) * 24;
			mover moveTo(dest, 0.05);
			wait 0.05;
			break;
		}
		mover moveTo(dest, 0.05);
		wait 0.05;
	}

	self unlink();
	mover delete();
	earthquake(0.6, 0.8, self.origin, 700);
	physicsExplosionSphere(self.origin, 250, 50, 1.5);
	self.mm_riding = undefined;
}

// ---------------------------------------------------------------------------
// Melee toys
// ---------------------------------------------------------------------------

ground_pound_set(on)
{
	self notify("mm_stop_pound");
	self mm_onoff("Ground Pound (RS in the air)", on);
	if (!on)
		return;

	self endon("mm_stop_pound");
	self endon("disconnect");
	for (;;)
	{
		wait 0.05;
		if (!isAlive(self) || self.mm_open || isDefined(self.mm_riding))
			continue;
		if (!self isOnGround() && self meleeButtonPressed())
			self ground_pound();
	}
}

ground_pound()
{
	self.mm_riding = true;
	ground = mm_ground(self.origin);
	mover = mm_spawn_mover(self.origin, (0, 0, 0));
	self playerLinkToDelta(mover, undefined, 1.0);
	mover moveTo(ground, 0.15);
	wait 0.15;
	self unlink();
	mover delete();

	earthquake(0.8, 1, ground, 900);
	physicsExplosionSphere(ground, 400, 100, 3);
	ai = mm_ai_near(ground, 350);
	for (i = 0; i < ai.size; i++)
	{
		if (ai[i].team == "allies")
			continue;
		push = vectorNormalize(mm_flat(ai[i].origin - ground)) * 500 + (0, 0, 500);
		thread mm_fling(ai[i], push, self);
	}
	mm_play_fx(mm_fx("thunder"), ground);
	wait 0.3;
	self.mm_riding = undefined;
}

force_field_set(on)
{
	self notify("mm_stop_field");
	self mm_onoff("Force Field", on);
	if (!on)
		return;

	self endon("mm_stop_field");
	self endon("disconnect");
	for (;;)
	{
		wait 0.1;
		if (!isAlive(self))
			continue;
		ai = mm_ai_near(self.origin, 170);
		for (i = 0; i < ai.size; i++)
		{
			if (ai[i].team == "allies")
				continue;
			push = vectorNormalize(mm_flat(ai[i].origin - self.origin)) * 700 + (0, 0, 300);
			mm_blood(ai[i].origin + (0, 0, 40));
			thread mm_fling(ai[i], push, self);
		}
	}
}

// ---------------------------------------------------------------------------
// Time & senses
// ---------------------------------------------------------------------------

matrix_set(on)
{
	level notify("mm_stop_matrix");
	self mm_onoff("Matrix Mode (slow-mo while aiming)", on);
	if (!on)
	{
		setTimeScale(mm_get("timescale", "level", 1));
		return;
	}

	level endon("mm_stop_matrix");
	slow = false;
	for (;;)
	{
		wait 0.05;
		aiming = isAlive(self) && self adsButtonPressed() && !self.mm_open;
		if (aiming && !slow)
		{
			setTimeScale(0.3);
			self setBlur(0.8, 0.2);
			slow = true;
		}
		else if (!aiming && slow)
		{
			setTimeScale(mm_get("timescale", "level", 1));
			self setBlur(0, 0.3);
			slow = false;
		}
	}
}

drunk_set(on)
{
	self notify("mm_stop_drunk");
	self mm_onoff("Drunk Mode", on);
	if (!on)
	{
		self setBlur(0, 0.5);
		self setDoubleVision(0, 0.5);
		return;
	}

	self endon("mm_stop_drunk");
	self endon("disconnect");
	self setDoubleVision(4, 1);
	for (t = 0;; t++)
	{
		wait 0.05;
		if (!isAlive(self) || self.mm_open)
			continue;
		angles = self getPlayerAngles();
		self setPlayerAngles((angles[0] + sin(t * 9) * 0.6, angles[1] + cos(t * 7) * 0.8, sin(t * 4) * 12));
		if (t % 40 == 0)
			self setBlur(randomFloatRange(0.5, 2.5), 1);
	}
}

// ---------------------------------------------------------------------------
// Parties
// ---------------------------------------------------------------------------

disco_set(on)
{
	level notify("mm_stop_disco");
	self mm_onoff("Disco Mode", on);
	if (!on)
	{
		resetSunLight();
		visionSetNaked(mm_level_vision(), 1);
		return;
	}

	level endon("mm_stop_disco");
	visions = maps\mod_menu\world::vision_names();
	for (t = 0;; t++)
	{
		setSunLight(randomFloatRange(0, 3), randomFloatRange(0, 3), randomFloatRange(0, 3));
		if (t % 8 == 0)
			visionSetNaked(visions[randomInt(visions.size)], 0.3);
		wait 0.25;
	}
}

nuke()
{
	self endon("disconnect");
	mm_announce("^1TACTICAL NUKE INCOMING");
	for (i = 3; i > 0; i--)
	{
		mm_announce("^1" + i);
		wait 1;
	}

	players = get_players();
	for (i = 0; i < players.size; i++)
		players[i] thread mm_flash((1, 1, 1), 3);
	earthquake(0.9, 4, self.origin, 100000);
	setSunLight(4, 3, 2);

	enemies = mm_enemies();
	for (i = 0; i < enemies.size; i++)
	{
		if (!isAlive(enemies[i]))
			continue;
		push = vectorNormalize(mm_flat(enemies[i].origin - self.origin)) * 400 + (0, 0, 700);
		thread mm_fling(enemies[i], push, self);
	}

	wait 3;
	resetSunLight();
	mm_announce("^7" + enemies.size + " enemies vaporized");
}

meteor_shower()
{
	self endon("disconnect");
	mm_announce("^3METEOR SHOWER");
	for (i = 0; i < 40; i++)
	{
		offset = (randomFloatRange(-1200, 1200), randomFloatRange(-1200, 1200), 0);
		if (distance(mm_flat(offset), (0, 0, 0)) < 250)
			offset = offset * 2;
		mm_explode(mm_ground(self.origin + offset + (0, 0, 400)), 260, self);
		wait randomFloatRange(0.1, 0.35);
	}
}

// Every enemy floats into the sky and explodes.
rapture()
{
	self endon("disconnect");
	mm_announce("^5THE RAPTURE");
	enemies = mm_enemies();
	for (i = 0; i < enemies.size; i++)
		enemies[i] thread rapture_one(self);
}

rapture_one(attacker)
{
	if (!isAlive(self))
		return;
	mover = mm_spawn_mover(self.origin, self.angles);
	self linkTo(mover);
	top = mm_ceiling(self.origin, 600) - (0, 0, 80);
	mover moveTo(top, randomFloatRange(2.5, 4), 1, 0.5);
	mover rotateYaw(720, 4);
	wait 4;
	if (isDefined(self))
	{
		self unlink();
		mm_explode(self.origin + (0, 0, 40), 120, attacker);
		thread mm_fling(self, mm_random_vec(300) + (0, 0, 200), attacker);
	}
	wait 0.1;
	mover delete();
}

blood_rain()
{
	mm_announce("^1IT'S RAINING BLOOD");
	for (i = 0; i < 80; i++)
	{
		players = get_players();
		for (p = 0; p < players.size; p++)
			mm_blood(players[p].origin + (randomFloatRange(-500, 500), randomFloatRange(-500, 500), randomFloatRange(100, 300)));
		wait 0.1;
	}
}

earthquake_now()
{
	earthquake(0.7, 6, self.origin, 5000);
	self iprintln("Brace yourself");
}

film_grain_set(on)
{
	self notify("mm_stop_grain");
	if (isDefined(self.mm_grain))
		self.mm_grain delete();
	self mm_onoff("Film Grain", on);
	if (!on)
		return;

	fx = mm_fx("grain_test");
	if (!isDefined(fx))
	{
		self iprintln("^1Grain effect is not loaded on this map");
		return;
	}

	self endon("mm_stop_grain");
	self endon("disconnect");
	self.mm_grain = mm_spawn_mover(self getEye(), (0, 0, 0));
	playFXOnTag(fx, self.mm_grain, "tag_origin");
	for (;;)
	{
		wait 0.05;
		self.mm_grain.origin = self getEye() + self mm_forward() * 50;
	}
}

// ---------------------------------------------------------------------------
// FX browser
// ---------------------------------------------------------------------------

fx_keys()
{
	keys = [];
	if (isDefined(level._effect))
	{
		all = getArrayKeys(level._effect);
		for (i = 0; i < all.size && keys.size < 60; i++)
			keys[keys.size] = all[i];
	}
	return keys;
}

fx_play(key)
{
	if (!isDefined(level._effect[key]))
		return;
	self.mm_fx_key = key;
	playFX(level._effect[key], self mm_aim_pos());
	self iprintln("Playing ^3" + key + "^7 (also loaded into FX Gun)");
}
