CLIENT_FLAG_NOCLIP = 1 << 0;
CLIENT_FLAG_UFO = 1 << 1;

ENTITY_FLAG_GODMODE = 1 << 0;

noclip()
{
    if (self.clientflags & CLIENT_FLAG_NOCLIP)
    {
        iprintln(&"GAME_NOCLIPOFF");
        self.clientflags &= ~CLIENT_FLAG_NOCLIP;
    }
    else
    {
        iprintln(&"GAME_NOCLIPON");
        self.clientflags |= CLIENT_FLAG_NOCLIP;
    }
}

ufo()
{
    if (self.clientflags & CLIENT_FLAG_UFO)
    {
        iprintln(&"GAME_UFOOFF");
        self.clientflags &= ~CLIENT_FLAG_UFO;
    }
    else
    {
        iprintln(&"GAME_UFOON");
        self.clientflags |= CLIENT_FLAG_UFO;
    }
}

isufo()
{
    return (self.clientflags & CLIENT_FLAG_UFO) != 0;
}

god()
{
    msg = &"";

    if (self.entityflags & ENTITY_FLAG_GODMODE)
    {
        msg = &"GAME_GODMODE_OFF";
        self.entityflags &= ~ENTITY_FLAG_GODMODE;
    }
    else
    {
        msg = &"GAME_GODMODE_ON";
        self.entityflags |= ENTITY_FLAG_GODMODE;
    }

    if (isPlayer(self))
    {
        iprintln(msg);
    }
}
