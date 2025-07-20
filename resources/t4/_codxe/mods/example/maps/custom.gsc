init()
{
	level thread onPlayerConnect();
}

onPlayerConnect()
{
	for (;;)
	{
		level waittill("connecting", player);
		player thread onPlayerSpawned();
	}
}

onPlayerSpawned()
{
	self endon("disconnect");

	for (;;)
	{
		self waittill("spawned_player");
		self thread WatchButtons();

		// Remove unlocalized errors for iprintln
		self setClientDvar("loc_warnings", 0);
	}
}

WatchButtons()
{
	self endon("death");
	self endon("disconnect");
	self endon("end_respawn");

	for (;;)
	{
		if (self UseButtonPressed())
		{
			self iprintln("Use button pressed");
			self toggle_godmode();
			wait 0.1;
		}
		else if (self FragButtonPressed())
		{
			self iprintln("Frag button pressed");
			self toggle_noclip();
			wait 0.1;
		}
		else if (self SecondaryOffhandButtonPressed())
		{
			self iprintln("SecondaryOffhand button pressed");
			self toggle_ufomode();
			wait 0.1;
		}
		else if (self AttackButtonPressed())
		{
			self iprintln("Attack button pressed");
			wait 0.1;
		}
		else if (self ADSButtonPressed())
		{
			self iprintln("ADS button pressed");
			wait 0.1;
		}
		else if (self MeleeButtonPressed())
		{
			self iprintln("Melee button pressed");
			wait 0.1;
		}
		else if (self JumpButtonPressed())
		{
			self iprintln("Jump button pressed");
			wait 0.1;
		}
		else if (self SprintButtonPressed())
		{
			self iprintln("Sprint button pressed");
			wait 0.1;
		}
		else if (self MoveForwardButtonPressed())
		{
			self iprintln("Move Forward button pressed");
			wait 0.1;
		}
		else if (self MoveBackButtonPressed())
		{
			self iprintln("Move Back button pressed");
			wait 0.1;
		}
		else if (self MoveLeftButtonPressed())
		{
			self iprintln("Move Left button pressed");
			wait 0.1;
		}
		else if (self MoveRightButtonPressed())
		{
			self iprintln("Move Right button pressed");
			wait 0.1;
		}
		wait 0.05;
	}
}

toggle_ufomode()
{
    if (self.ufo)
    {
        self.ufo = false;
        self iprintln("ufomode OFF");
    }
    else
    {
        self.ufo = true;
        self iprintln("ufomode ON");
    }
}

toggle_noclip()
{
    if (self.noclip)
    {
        self.noclip = false;
        self iprintln("noclip OFF");
    }
    else
    {
        self.noclip = true;
        self iprintln("noclip ON");
    }
}

toggle_godmode()
{
    if (self.god)
    {
        self.god = false;
        iprintln("godmode OFF");
    }
    else
    {
        self.god = true;
        iprintln("godmode ON");
    }
}
