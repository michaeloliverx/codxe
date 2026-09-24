/*
	Co-op player management (split-screen / online co-op campaign and zombies).
*/

#include maps\_utility;
#include common_scripts\utility;
#include maps\mod_menu\util;

player_label(player)
{
	name = "Player";
	if (isDefined(player.playername))
		name = player.playername;
	if (player == self)
		name = name + " (you)";
	return mm_short(name, 20);
}

select_player(player)
{
	self.mm_target = player;
	self maps\mod_menu\core::mm_enter("player_opts");
}

target()
{
	if (!isDefined(self.mm_target) || !isPlayer(self.mm_target))
	{
		self iprintln("^1That player left");
		return undefined;
	}
	return self.mm_target;
}

bring_here()
{
	p = self target();
	if (!isDefined(p))
		return;
	p setOrigin(self.origin + self mm_forward() * 60);
	self iprintln("Teleported " + self player_label(p) + " to you");
}

go_to()
{
	p = self target();
	if (!isDefined(p))
		return;
	self setOrigin(p.origin - anglesToForward(p getPlayerAngles()) * 60);
}

give_god()
{
	p = self target();
	if (!isDefined(p))
		return;
	p.god = !p.god;
	if (p.god)
		p iprintln("God Mode [^2ON^7]");
	else
		p iprintln("God Mode [^1OFF^7]");
	self mm_onoff("God for " + self player_label(p), p.god);
}

give_menu()
{
	p = self target();
	if (!isDefined(p))
		return;
	if (p == self)
		return;
	p.mm_access = !p.mm_access;
	if (p.mm_access)
		p iprintlnbold("You were given the ^5CoD Xe Menu^7 - hold LT + press RS");
	self mm_onoff("Menu access for " + self player_label(p), p.mm_access);
}

launch()
{
	p = self target();
	if (!isDefined(p))
		return;
	p iprintlnbold("^3YEET");
	p thread maps\mod_menu\fun::human_cannonball();
}

revive()
{
	p = self target();
	if (!isDefined(p))
		return;
	if (!p maps\_laststand::player_is_in_laststand())
	{
		self iprintln("They are not down");
		return;
	}
	p maps\_laststand::revive_force_revive(self);
}

give_points()
{
	p = self target();
	if (!isDefined(p))
		return;
	p.score += 10000;
	p iprintln("^2+10000 ^7points from " + self player_label(self));
}
