#include maps\codxe_utility;

init()
{
    level.player setup_player();
}

setup_player()
{
    iprintln("Welcome " + self.name + "!");
    self god();
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
            self ufo();
            break;
        case "dpad_right":
            self noclip();
            break;
        default:
            break;
        }
    }
}
