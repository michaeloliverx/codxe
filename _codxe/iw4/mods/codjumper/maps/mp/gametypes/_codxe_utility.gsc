CLIENT_FLAG_NOCLIP = 1 << 0;
CLIENT_FLAG_UFO = 1 << 1;

ENTITY_FLAG_GODMODE = 1 << 0;

Noclip()
{
    if (self.clientflags & CLIENT_FLAG_NOCLIP)
    {
        self iprintln(&"GAME_NOCLIPOFF");
        self.clientflags &= ~CLIENT_FLAG_NOCLIP;
    }
    else
    {
        self iprintln(&"GAME_NOCLIPON");
        self.clientflags |= CLIENT_FLAG_NOCLIP;
    }
}

UFO()
{
    if (self.clientflags & CLIENT_FLAG_UFO)
    {
        self iprintln(&"GAME_UFOOFF");
        self.clientflags &= ~CLIENT_FLAG_UFO;
    }
    else
    {
        self iprintln(&"GAME_UFOON");
        self.clientflags |= CLIENT_FLAG_UFO;
    }
}

IsUFO()
{
    return (self.clientflags & CLIENT_FLAG_UFO) != 0;
}

God()
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
        self iprintln(msg);
    }
}
