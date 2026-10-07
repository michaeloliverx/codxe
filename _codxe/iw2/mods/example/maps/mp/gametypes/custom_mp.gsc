main()
{
    level thread onPlayerConnect();
}

onPlayerConnect()
{
	for(;;)
	{
		level waittill("connecting", player);

		player thread onPlayerSpawned();
	}
}

onPlayerSpawned()
{
	self endon("disconnect");
	
	for(;;)
	{
		self waittill("spawned_player");
		
		self iprintlnbold("^2Welcome: " + self.name);
		setcvar("g_speed", 500);
		setcvar( "bg_fallDamageMinHeight", 99998 );
		setcvar( "bg_fallDamageMaxHeight", 99999 );
	}
}
