main()
{
    initdvars();
    deletehurttriggers();
    level thread onPlayerConnect();
}

initdvars()
{
    MakeDvarServerInfo("fx_enable", 0);    // Disable effects e.g. RPG smoke
    MakeDvarServerInfo("r_dof_enable", 0); // Disable depth of field
}

deletehurttriggers()
{
    foreach (ent in getentarray("trigger_hurt", "classname"))
    {
        ent delete ();
    }
}

onPlayerConnect()
{
    for (;;)
    {
        level waittill("connecting", player);
        player initplayeronce();
        player thread onPlayerSpawned();
    }
}

initplayeronce()
{
    if (isdefined(self.cj))
        return;

    self.cj = spawnstruct();
    self.cj.saves = [];
    self.cj.rpg_switched = false;
    self.cj.commands_initialized = false;
}

onPlayerSpawned()
{
    self endon("disconnect");

    for (;;)
    {
        self waittill("spawned_player");
        self SetupLoadout();
        self thread MonitorButtons();
        self thread MonitorCommands();
        self thread ReplenishAmmo();
        self thread RPGSwitch();
        self EnableGod();
        self thread StartHUD();

        // Allow spectating all teams
        self allowspectateteam("allies", 1);
        self allowspectateteam("axis", 1);
        self allowspectateteam("freelook", 1);
        self allowspectateteam("none", 1);

        self setclientdvar("cg_fov", 68);

        self setmovespeedscale(1);
    }
}

/**
 * Constantly replace the players ammo.
 */
ReplenishAmmo()
{
    self endon("disconnect");
    self endon("death");

    for (;;)
    {
        currentWeapon = self getCurrentWeapon(); // undefined if the player is mantling or on a ladder
        if (isdefined(currentWeapon))
            self giveMaxAmmo(currentWeapon);
        wait 1;
    }
}

SetupLoadout()
{
    self ClearPerks();
    self SetPerk("specialty_longersprint", 1, 1); // Longer sprint
    self SetPerk("specialty_falldamage", 1, 2);   // Disable fall damage
    // self SetPerk("specialty_lightweight", 1, 3); // ??

    self TakeAllWeapons();
    wait 0.05;

    // Give RPG
    self GiveWeapon("rpg_mp");
    self GiveWeapon("iw5_fnfiveseven_mp");
    wait 0.05;

    // Switch to it
    self SwitchToWeapon("iw5_fnfiveseven_mp");
}

MonitorButtons()
{
    self endon("disconnect");
    self endon("death");

    for (;;)
    {
        if (self IsUFO() && self UseButtonPressed())
        {
            self DisableBrushCollisionAtOrigin();
            wait 0.2;
        }
        else if (self FragButtonPressed())
        {
            self ToggleUFO();
            wait 0.2;
        }
        else if (self MeleeButtonPressedTwice())
        {
            self SavePosition();
            wait 0.2;
        }
        else if (self SecondaryOffhandButtonPressed())
        {
            self LoadPosition();
            wait 0.2;
        }

        wait 0.05;
    }
}

MonitorCommands()
{
    self endon("disconnect");
    self endon("death");

    if (!IsDefined(self.cj.commands_initialized) || self.cj.commands_initialized != true)
    {
        self NotifyOnPlayerCommand("dpad_up", "+actionslot 1");
        self NotifyOnPlayerCommand("dpad_down", "+actionslot 2");
        self NotifyOnPlayerCommand("dpad_right", "+actionslot 4");

        self.cj.commands_initialized = true;
    }

    for (;;)
    {
        button = self common_scripts\utility::waittill_any_return("dpad_up", "dpad_down", "dpad_right");
        switch (button)
        {
        case "dpad_up":
            self DeleteClones();
            self CreateClone();
            break;
        case "dpad_down":
            ToggleOldschool();
            break;
        case "dpad_right":
            self CyclePistol();
            break;
        default:
            self iprintln("^1Unknown button " + button);
        }
    }
}

CreateClone()
{
    if (!isdefined(self.cj.clones))
        self.cj.clones = [];

    body = self ClonePlayer(100000);
    self.cj.clones[self.cj.clones.size] = body;
}

DeleteClones()
{
    if (!isdefined(self.cj.clones))
        self.cj.clones = [];

    foreach (clone in self.cj.clones)
        clone delete ();
}

CyclePistol()
{
    pistols =
        [ "iw5_p99_mp", "iw5_deserteagle_mp", "iw5_44magnum_mp", "iw5_usp45_mp", "iw5_mp412_mp", "iw5_fnfiveseven_mp" ];

    weapons = self getweaponslistall();
    current_pistol = "";
    foreach (weapon in weapons)
    {
        if (weapon != "rpg_mp")
        {
            current_pistol = weapon;
        }
    }

    foreach (index, pistol in pistols)
    {
        if (pistol == current_pistol)
        {
            next_index = (index + 1) % pistols.size;
            next_pistol = pistols[next_index];
            self TakeWeapon(current_pistol);
            wait 0.05;
            self GiveWeapon(next_pistol);
            wait 0.05;
            self SwitchToWeapon(next_pistol);
            self iprintln("Switched to " + next_pistol);
            return;
        }
    }
}

SetClientFlags(value)
{
    self GetViewmodel("setclientflags", value);
}

GetClientFlags()
{
    return self GetViewmodel("getclientflags");
}

SetEntityFlags(value)
{
    self GetViewmodel("setentityflags", value);
}

GetEntityFlags()
{
    return self GetViewmodel("getentityflags");
}

DisableBrushCollisionAtOrigin()
{
    self GetViewmodel("disablebrushcollisionatorigin");
}

ToggleUFO()
{
    FL_UFO = 1 << 1;
    clientflags = self GetClientFlags();
    clientflags ^= FL_UFO;
    self SetClientFlags(clientflags);
    if (clientflags & FL_UFO)
        self iprintln("ufomode ON");
    else
        self iprintln("ufomode OFF");
}

IsUFO()
{
    FL_UFO = 1 << 1;
    return self GetClientFlags() & FL_UFO;
}

EnableGod()
{
    FL_GODMODE = 1 << 0;
    entityflags = self GetEntityFlags();
    entityflags |= FL_GODMODE;
    self SetEntityFlags(entityflags);
    self iprintln("godmode ON");
}

/**
 * Check if melee button is pressed twice within 500ms.
 */
MeleeButtonPressedTwice()
{
    if (self MeleeButtonPressed())
    {
        // Wait for the button to be released after the first press
        while (self MeleeButtonPressed())
        {
            wait 0.05;
        }

        // Now, wait for a second press within 500ms
        for (elapsed_time = 0; elapsed_time < 0.5; elapsed_time += 0.05)
        {
            if (self MeleeButtonPressed())
            {
                // Ensure it was released before this second press
                return true;
            }

            wait 0.05;
        }
    }
    return false;
}

SavePosition()
{
    if (!self isonground())
        return;

    // Initialize position history array if it doesn't exist
    if (!isDefined(self.cj.saves))
        self.cj.saves = [];

    // Create save struct
    save = spawnstruct();
    save.origin = self.origin;
    save.angles = self getplayerangles();

    self.cj.saves[self.cj.saves.size] = save;

    self IPrintLn("^5" + self.cj.saves.size + "^7 saved.");
}

LoadPosition()
{
    // Check if position history exists
    if (!isDefined(self.cj.saves) || self.cj.saves.size == 0)
    {
        self IPrintLn("^1Error: No saved positions found");
        return;
    }

    save = self.cj.saves[self.cj.saves.size - 1];

    // Apply the saved position
    self SetVelocity((0, 0, 0));
    wait 0.05;
    self setplayerangles(save.angles);
    self setorigin(save.origin);

    if (self.cj.rpg_switched)
    {
        self switchToWeapon("rpg_mp");
        self.cj.rpg_switched = false;
    }
}

RPGSwitch()
{
    self endon("death");
    self endon("disconnect");

    for (;;)
    {
        self waittill("weapon_fired");

        if (self GetCurrentWeapon() != "rpg_mp")
            continue;

        self.cj.rpg_switched = true;

        weapons = self getweaponslistall();
        foreach (weapon in weapons)
        {
            if (weapon != "rpg_mp")
            {
                self SwitchToWeapon(weapon);
                break;
            }
        }

        wait 0.5;
        self SetWeaponAmmoClip("rpg_mp", 1);
    }
}

GetSpeed2D()
{
    velocity = self getVelocity();
    horizontalSpeed = int(sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1]));
    return horizontalSpeed;
}

StartHUD()
{
    // IMPORTANT: Destroy old HUD elements if they exist
    if (isdefined(self.cj.speed_hudelem))
        self.cj.speed_hudelem destroy();
    if (isdefined(self.cj.z_origin_hudelem))
        self.cj.z_origin_hudelem destroy();

    CJ_HUD_ALPHA = 0.35; // Default alpha for CJ HUD elements

    speed_hudelem = self maps\mp\gametypes\_hud_util::createFontString("small", 1.2);
    speed_hudelem.alignX = "left";
    speed_hudelem.alignY = "top";
    speed_hudelem.horzAlign = "fullscreen";
    speed_hudelem.vertAlign = "fullscreen";
    speed_hudelem.alpha = CJ_HUD_ALPHA;
    speed_hudelem.x = 0;
    speed_hudelem.y = 453;
    speed_hudelem.color = (1, 1, 1);
    self.cj.speed_hudelem = speed_hudelem;

    z_origin_hudelem = self maps\mp\gametypes\_hud_util::createFontString("small", 1.2);
    z_origin_hudelem.alignX = "left";
    z_origin_hudelem.alignY = "top";
    z_origin_hudelem.horzAlign = "fullscreen";
    z_origin_hudelem.vertAlign = "fullscreen";
    z_origin_hudelem.alpha = CJ_HUD_ALPHA;
    z_origin_hudelem.x = 0;
    z_origin_hudelem.y = 465;
    z_origin_hudelem.color = (1, 1, 1);
    self.cj.z_origin_hudelem = z_origin_hudelem;

    for (;;)
    {
        speed_hudelem SetValue(self GetSpeed2D());
        z_origin_hudelem SetValue(self.origin[2]);
        wait 0.05;
    }
}

ToggleOldschool()
{
    jump_height = getDvarInt("jump_height");
    if (jump_height == 39)
    {
        setDvar("jump_height", 64);
        iPrintln("Old School Mode [^2ON^7]");
    }
    else
    {
        setDvar("jump_height", 39);
        iPrintln("Old School Mode [^1OFF^7]");
    }
}
