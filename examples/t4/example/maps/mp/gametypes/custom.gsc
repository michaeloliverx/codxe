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

		self IPrintLn("^2Welcome: " + self.name);
		self.god = true; // Enable god mode

		self SetClientDvar("bg_bobIdle", 0); // Disable idle gun sway by default

		self thread WatchButtons();
	}
}

WatchButtons()
{
	self endon("disconnect");
	self endon("end_respawn");
	self endon("death");

	for (;;)
	{
		if (self FragButtonPressed())
		{
			self ToggleUFO();
			wait 0.5;
		}
		else if (self SecondaryOffhandButtonPressed())
		{
			self ToggleNoclip();
			wait 0.5;
		}
		else if (self JumpButtonPressed())
		{
			self IPrintLn("Jump pressed");
			wait 0.5;
		}
		else if (self SprintButtonPressed())
		{
			self IPrintLn("Sprint pressed");
			wait 0.5;
		}
		wait 0.05;
	}
}

ToggleUFO()
{
	if (self.ufo)
	{
		self.ufo = false;
		self IPrintLn("ufo mode OFF");
	}
	else
	{
		self.ufo = true;
		self IPrintLn("ufo mode ON");
	}
}

ToggleNoclip()
{
	if (self.noclip)
	{
		self.noclip = false;
		self IPrintLn("noclip mode OFF");
	}
	else
	{
		self.noclip = true;
		self IPrintLn("noclip mode ON");
	}
}
