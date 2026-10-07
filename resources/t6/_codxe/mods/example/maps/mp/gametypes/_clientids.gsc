init()
{
    level.clientid = 0;
    level thread onPlayerConnect();
}

onPlayerConnect()
{
    for (;;)
    {
        level waittill("connecting", player);

        player.clientid = level.clientid;
        level.clientid++;

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

        self iprintlnbold(rainbow_text("Welcome to the Example Mod !"));
    }
}

rainbow_text(str)
{
    if (!isDefined(str))
        return "";

    colors = [];
    colors[colors.size] = "^1"; // Red
    colors[colors.size] = "^2"; // Green
    colors[colors.size] = "^3"; // Yellow
    colors[colors.size] = "^4"; // Blue
    colors[colors.size] = "^5"; // Cyan
    colors[colors.size] = "^6"; // Pink
    colors[colors.size] = "^7"; // White

    rainbow = "";
    colorCount = colors.size;
    colorIndex = 0;

    for (i = 0; i < str.size; i++)
    {
        ch = str[i];
        if (ch == " ")
        {
            rainbow += ch;
            continue;
        }

        color = colors[colorIndex % colorCount];
        rainbow += color + ch;
        colorIndex++;
    }

    return rainbow;
}
