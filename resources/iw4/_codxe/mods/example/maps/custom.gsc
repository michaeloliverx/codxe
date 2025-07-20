main()
{
    level.player setup_player();
}

setup_player()
{
    iprintln("Welcome " + self.name + "!");
    self toggle_godmode();
    self thread watch_buttons();
}

watch_buttons()
{
    self endon("death");
    self endon("disconnect");

    self notifyOnPlayerCommand("dpad_down", "+actionslot 2");
    self notifyOnPlayerCommand("dpad_left", "+actionslot 3");
    self notifyOnPlayerCommand("dpad_right", "+actionslot 4");

    for (;;)
    {
        button = self common_scripts\utility::waittill_any_return("dpad_down", "dpad_right");
        switch (button)
        {
        case "dpad_down":
            self toggle_ufomode();
            break;
        case "dpad_right":
            self toggle_noclip();
            break;
        default:
            break;
        }
    }
}

toggle_ufomode()
{
    if (self.ufo)
    {
        self.ufo = false;
        iprintln("ufomode OFF");
    }
    else
    {
        self.ufo = true;
        iprintln("ufomode ON");
    }
}

toggle_noclip()
{
    if (self.noclip)
    {
        self.noclip = false;
        iprintln("noclip OFF");
    }
    else
    {
        self.noclip = true;
        iprintln("noclip ON");
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
