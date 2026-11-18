noclip()
{
    if (self.clientflags & 1)
    {
        iprintln(&"GAME_NOCLIPOFF");
        self.clientflags &= ~1;
    }
    else
    {
        iprintln(&"GAME_NOCLIPON");
        self.clientflags |= 1;
    }
}

ufo()
{
    if (self.clientflags & 2)
    {
        iprintln(&"GAME_UFOOFF");
        self.clientflags &= ~2;
    }
    else
    {
        iprintln(&"GAME_UFOON");
        self.clientflags |= 2;
    }
}

god()
{
    FL_GODMODE = 1 << 0;
    msg = &"";

    if (self.entityflags & FL_GODMODE)
    {
        msg = &"GAME_GODMODE_OFF";
        self.entityflags &= ~FL_GODMODE;
    }
    else
    {
        msg = &"GAME_GODMODE_ON";
        self.entityflags |= FL_GODMODE;
    }

    if (isPlayer(self))
    {
        iprintln(msg);
    }
}
