init()
{
    setup_global_dvars();
    level thread onPlayerConnect();
}

setup_global_dvars()
{
    setDvar("player_sprintUnlimited", 1);
    setDvar("jump_slowdownEnable", 0);

    // Remove fall damage
    setDvar("bg_fallDamageMaxHeight", 9999);
    setDvar("bg_fallDamageMinHeight", 9998);

    setDvar("bg_bobMax", 0);
    setDvar("bg_swayScale", 0);
    setDvar("r_motionblur_enable ", 0);
}

onPlayerConnect()
{
    level endon("game_ended");

    for (;;)
    {
        level waittill("connecting", player);

        player thread onPlayerSpawned();
    }
}

onPlayerSpawned()
{
    self endon("disconnect");
    level endon("game_ended");

    for (;;)
    {
        self waittill("spawned_player");

        self togglegod();
        self setup_loadout();
        self thread button_monitor();
    }
}

setup_loadout()
{
    self takeallweapons();
    self giveweapon("gold_p99_mp");
    self giveweapon("rpg_mp");
    self giveweapon("at4_mp");
    wait 0.05;
    self switchtoweapon("gold_p99_mp");
}

button_monitor()
{
    level endon("game_ended");
    self endon("disconnect");
    self endon("spawned_player");

    for (;;)
    {
        wait 0.05;

        if (self fragbuttonpressed())
        {
            self toggleufo();
            wait 0.2;
        }

        else if (self getufo() && self usebuttonpressed())
        {
            self iprintln("Secondary Offhand Button Pressed");
            // Example action: Disable brush collision at the player's position
            point = self.origin - (0, 0, 1);
            disablebrushcollisioncontainingpoint(point);
            wait 0.2;
        }

        else if (self meleebuttonpressed())
        {
            self save_position();
            wait 0.2;
        }

        else if (self usebuttonpressed())
        {
            self load_position();
            wait 0.2;
        }
    }
}

save_position()
{
    if (!self isonground())
        return;

    save = spawnstruct();
    save.origin = self.origin;
    save.angles = self getplayerangles();

    self.saved_pos = save;

    self iprintln("Position saved");
}

load_position()
{
    self freezecontrols(true);
    wait 0.05;

    save = self.saved_pos;

    self setplayerangles(save.angles);
    self setorigin(save.origin);

    wait 0.05;
    self freezecontrols(false);
}
