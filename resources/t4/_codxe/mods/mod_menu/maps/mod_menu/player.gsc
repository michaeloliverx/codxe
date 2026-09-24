/*
	Player (self) mods. god/noclip/ufo are client fields added by CoD Xe.
*/

#include maps\_utility;
#include common_scripts\utility;
#include maps\mod_menu\util;

// ---------------------------------------------------------------------------
// CoD Xe client fields
// ---------------------------------------------------------------------------

god_get()
{
	return self.god;
}

god_set(on)
{
	self.god = on;
	self mm_onoff("God Mode", on);
}

noclip_get()
{
	return self.noclip;
}

noclip_set(on)
{
	self.noclip = on;
	self mm_onoff("Noclip", on);
}

ufo_get()
{
	return self.ufo;
}

ufo_set(on)
{
	self.ufo = on;
	self mm_onoff("UFO Mode", on);
}

// ---------------------------------------------------------------------------
// Survival
// ---------------------------------------------------------------------------

demigod_set(on)
{
	self notify("mm_stop_demigod");
	self mm_onoff("Demigod (feel hits, never die)", on);
	if (!on)
		return;

	self endon("mm_stop_demigod");
	self endon("disconnect");
	for (;;)
	{
		if (isAlive(self) && self.health < self.maxhealth)
			self.health = self.maxhealth;
		wait 0.05;
	}
}

invisible_set(on)
{
	self.ignoreme = on;
	if (on)
		self hide();
	else
		self show();
	self mm_onoff("Invisible (AI ignore you)", on);
}

max_health()
{
	self.maxhealth = 1000;
	self.health = 1000;
	self iprintln("Health set to ^21000");
}

// ---------------------------------------------------------------------------
// Ammo
// ---------------------------------------------------------------------------

infinite_ammo_set(on)
{
	self notify("mm_stop_ammo");
	self mm_onoff("Infinite Ammo", on);
	if (!on)
		return;

	self endon("mm_stop_ammo");
	self endon("disconnect");
	for (;;)
	{
		if (isAlive(self))
		{
			weapons = self getWeaponsList();
			for (i = 0; i < weapons.size; i++)
			{
				weapon = weapons[i];
				if (weapon == "none")
					continue;
				self giveMaxAmmo(weapon);
				self setWeaponAmmoClip(weapon, weaponClipSize(weapon));
			}
		}
		wait 0.1;
	}
}

// ---------------------------------------------------------------------------
// Movement
// ---------------------------------------------------------------------------

speed_set(value)
{
	self setMoveSpeedScale(value);
}

super_jump_set(on)
{
	if (on)
	{
		mm_set_dvar("jump_height", "999");
		mm_set_dvar("bg_fallDamageMinHeight", "9998");
		mm_set_dvar("bg_fallDamageMaxHeight", "9999");
	}
	else
	{
		mm_restore_dvar("jump_height", "39");
		mm_restore_dvar("bg_fallDamageMinHeight", "128");
		mm_restore_dvar("bg_fallDamageMaxHeight", "300");
	}
	self mm_onoff("Super Jump", on);
}

no_fall_damage_set(on)
{
	if (on)
	{
		mm_set_dvar("bg_fallDamageMinHeight", "9998");
		mm_set_dvar("bg_fallDamageMaxHeight", "9999");
	}
	else
	{
		mm_restore_dvar("bg_fallDamageMinHeight", "128");
		mm_restore_dvar("bg_fallDamageMaxHeight", "300");
	}
	self mm_onoff("No Fall Damage", on);
}

sprint_set(on)
{
	if (on)
		mm_set_dvar("player_sprintUnlimited", "1");
	else
		mm_restore_dvar("player_sprintUnlimited", "0");
	self mm_onoff("Unlimited Sprint", on);
}

// ---------------------------------------------------------------------------
// Camera
// ---------------------------------------------------------------------------

third_person_set(on)
{
	self setClientDvar("cg_thirdPerson", on);
	self setClientDvar("cg_thirdPersonRange", 120);
	self mm_onoff("Third Person", on);
}

fov_set(value)
{
	self setClientDvar("cg_fov", value);
}

// ---------------------------------------------------------------------------
// Teleports
// ---------------------------------------------------------------------------

teleport_crosshair()
{
	trace = self mm_trace();
	pos = trace["position"] + trace["normal"] * 24;
	self setOrigin(mm_ground(pos));
	self iprintln("Teleported");
}

save_position()
{
	self.mm_saved_origin = self.origin;
	self.mm_saved_angles = self getPlayerAngles();
	self iprintln("Position ^2saved");
}

load_position()
{
	if (!isDefined(self.mm_saved_origin))
	{
		self iprintln("^1No saved position");
		return;
	}
	self setOrigin(self.mm_saved_origin);
	self setPlayerAngles(self.mm_saved_angles);
	self iprintln("Position ^2loaded");
}

teleport_to_enemy()
{
	enemies = mm_enemies();
	if (enemies.size == 0)
	{
		self iprintln("^1No enemies alive");
		return;
	}
	target = enemies[randomInt(enemies.size)];
	behind = target.origin - anglesToForward(target.angles) * 60;
	self setOrigin(behind);
	self setPlayerAngles(vectorToAngles(target.origin - behind));
	self iprintln("Behind you...");
}

// Teleport just under the sky, then keep fall damage off until landing.
skydive()
{
	self endon("disconnect");
	top = mm_ceiling(self.origin, 12000) - (0, 0, 72);
	if (top[2] - self.origin[2] < 256)
	{
		self iprintln("^1No room above you");
		return;
	}
	mm_set_dvar("bg_fallDamageMinHeight", "9998");
	mm_set_dvar("bg_fallDamageMaxHeight", "9999");
	self setOrigin(top);
	self iprintlnbold("^5GERONIMO!");
	wait 0.5;
	while (isAlive(self) && !self isOnGround())
		wait 0.05;
	earthquake(0.5, 0.8, self.origin, 600);
	physicsExplosionSphere(self.origin, 220, 60, 1.5);
	wait 0.5;
	if (!mm_get("no_fall", "level", false) && !mm_get("superjump", "level", false))
	{
		mm_restore_dvar("bg_fallDamageMinHeight", "128");
		mm_restore_dvar("bg_fallDamageMaxHeight", "300");
	}
}
