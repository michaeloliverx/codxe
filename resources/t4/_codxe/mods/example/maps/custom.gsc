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
	}
}

WatchButtons()
{
	self endon("disconnect");
	self endon("end_respawn");
	self endon("death");

	for (;;)
	{
		if (self UseButtonPressed())
		{
			self iprintln("Use button pressed");
			wait 0.1;
		}
		else if (self FragButtonPressed())
		{
			self iprintln("Frag button pressed");
			wait 0.1;
		}
		else if (self SecondaryOffhandButtonPressed())
		{
			self iprintln("SecondaryOffhand button pressed");
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
