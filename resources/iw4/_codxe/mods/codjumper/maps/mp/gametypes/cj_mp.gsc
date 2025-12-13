#include maps\mp\gametypes\_codxe_utility;
#include maps\mp\_utility;

init()
{
    precachemodel("viewmodel_base_viewhands");

    // Unless this is set then SP maps spawn with no weapons
    // https://github.com/shit-ware/IW4/blob/master/common_scripts/utility.gsc#L2080
    level.isSP = false;

    DeleteUnwantedEntities();

    SetDvar("scr_war_timelimit", 0);

    SetDvar("g_hardcore", 1);        // Hardcore HUD
    SetDvar("scr_game_forceuav", 0); // Disable compass
    SetDvar("ui_drawCrosshair", 0);

    SetDvar("r_fog", 0);

    SetDvar("player_sprintUnlimited", 1);
    SetDvar("jump_slowdownEnable", 0);

    SetDvar("bg_fallDamageMinHeight", 9998);
    SetDvar("bg_fallDamageMaxHeight", 9999);

    SetDvar("testClients_doAttack", 0);
    SetDvar("testClients_doCrouch", 0);
    SetDvar("testClients_doMove", 0);

    maps\mp\gametypes\menu::init();

    level thread onPlayerConnect();
}

DeleteUnwantedEntities()
{
    ents = getentarray();
    foreach (ent in ents)
    {
        // Death zones
        if (ent.classname == "trigger_hurt")
            ent delete ();
        // Radiation zones
        else if (isdefined(ent.targetname) && ent.targetname == "radiation")
            ent delete ();
    }
}

onPlayerConnect()
{
    for (;;)
    {
        level waittill("connecting", player);

        // Special handling for bots
        if (IsDefined(player.pers["isBot"]))
            continue;

        player InitPlayerOnce();

        player thread onPlayerSpawned();
    }
}

is_sp_mapname()
{
    return getsubstr(getdvar("mapname"), 0, 2) != "mp_";
}

onPlayerSpawned()
{
    self endon("disconnect");
    for (;;)
    {
        self waittill("spawned_player");

        self iprintlnbold("Welcome " + self.name + " to CodJumper!");
        self InitClientDvars();
        self God();
        self SetupLoadout();
        self thread ReplenishAmmo();
        self thread RPGSwitch();
        self thread MonitorButtons();
        self thread MonitorPlayerCommands();
        self thread maps\mp\gametypes\cj_hud::StartHUD();

        // On singleplayer maps use the base viewhands included in common_mp
        if (is_sp_mapname())
        {
            self setViewmodel("viewmodel_base_viewhands");
        }
    }
}

InitPlayerOnce()
{
    self endon("disconnect");
    self endon("death");

    if (IsDefined(self.cj))
        return;

    self iprintlnbold("Initializing CodJumper for " + self.name);

    self.cj = spawnstruct();
    self.cj.rpg_switched = false;
    self.cj.saves = [];
}

InitClientDvars()
{
    self SetClientDvar("bg_bobMax", 0);                                      // Remove view bob
    self SetClientDvar("cg_fov", 70);                                        //
    self SetClientDvar("loc_warnings", 0);                                   // Disable unlocalized text warnings
    self SetClientDvar("cg_drawSpectatorMessages", 0);                       // Disable spectator button icons
    self SetClientDvar("player_spectateSpeedScale", 1.5);                    // Increase spectator speed
    self SetClientDvar("player_view_pitch_up", 89.9);                        // Allow looking straight up
    self SetClientDvar("fx_enable", 0);                                      // Disable FX (RPG smoke etc)
    self SetClientDvar("aim_automelee_range", 0);                            // Remove melee lunge on enemy players
    self SetClientDvars("aim_slowdown_enabled", 0, "aim_lockon_enabled", 0); // Remove aim assist on enemy players
    self SetClientDvars("cg_overheadNamesSize", 0, "cg_overheadRankSize",
                        0); // Remove overhead names and ranks on enemy players

    // make on screen messages have a shorter duration
    for (i = 0; i < 4; i++)
    {
        self setClientDvars("con_gameMsgWindow" + i + "FadeInTime", 0.15, "con_gameMsgWindow" + i + "FadeOutTime", 0.15,
                            "con_gameMsgWindow" + i + "MsgTime", .85);
    }
}

SetupLoadout()
{
    self ClearPerks();
    self TakeAllWeapons();
    wait 0.05;

    self setPerk("specialty_lightweight");

    self GiveWeapon("deserteaglegold_mp");
    self GiveWeapon("rpg_mp");
    wait 0.05;

    self SwitchToWeapon("deserteaglegold_mp");
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

ReplenishAmmo()
{
    self endon("disconnect");
    self endon("death");

    for (;;)
    {
        currentWeapon = self getCurrentWeapon(); // undefined if the player is mantling or on a ladder
        if (IsDefined(currentWeapon))
            self giveMaxAmmo(currentWeapon);
        wait 1;
    }
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

UseButtonPressedTwice()
{
    if (self UseButtonPressed())
    {
        // Wait for the button to be released after the first press
        while (self UseButtonPressed())
        {
            wait 0.05;
        }

        // Now, wait for a second press within 500ms
        for (elapsed_time = 0; elapsed_time < 0.5; elapsed_time += 0.05)
        {
            if (self UseButtonPressed())
            {
                // Ensure it was released before this second press
                return true;
            }

            wait 0.05;
        }
    }
    return false;
}

MonitorButtons()
{
    self endon("disconnect");
    self endon("death");

    for (;;)
    {
        if (!(self maps\mp\gametypes\menu::isMenuOpen()))
        {
            if (self IsUFO() && self UseButtonPressed())
            {
                self DisablePlayerClipOnIntersectingBrushes();
                wait 0.2;
            }
            else if (self UseButtonPressedTwice())
            {
                self maps\mp\gametypes\menu::menuAction("OPEN");
                wait 0.2;
            }
            else if (self FragButtonPressed())
            {
                self UFO();
                wait 0.2;
            }
            else if (self MeleeButtonPressedTwice())
            {
                self SavePosition();
                wait 0.2;
            }
            else if (!self IsUFO() && self SecondaryOffhandButtonPressed())
            {
                if (self.cj.saves.size == 0)
                    self IPrintLn("No positions saved yet.");
                else
                {
                    // Load last position immediately
                    self LoadPosition();
                    savenum = self.cj.saves.size;
                    // If user holds the button, load previous positions when pressing use
                    while (self SecondaryOffhandButtonPressed())
                    {
                        if (self UseButtonPressed())
                        {
                            savenum = savenum - 1;
                            if (savenum < 1)
                                savenum = self.cj.savenum;
                            self LoadPosition(savenum);
                        }
                        wait.1;
                    }
                }

                wait.2;
            }
        }
        else
        {
            if (self UseButtonPressed())
            {
                self maps\mp\gametypes\menu::menuAction("SELECT");
                wait.2;
            }
            else if (self MeleeButtonPressed())
            {
                self maps\mp\gametypes\menu::menuAction("BACK");
                wait.2;
            }
            else if (self ADSButtonPressed())
            {
                self maps\mp\gametypes\menu::menuAction("UP");
                wait.2;
            }
            else if (self AttackButtonPressed())
            {
                self maps\mp\gametypes\menu::menuAction("DOWN");
                wait.2;
            }
        }

        wait 0.05;
    }
}

MonitorPlayerCommands()
{
    self endon("disconnect");
    self endon("death");

    if (!IsDefined(self.cj_commands_initialized) || self.cj_commands_initialized != true)
    {
        self NotifyOnPlayerCommand("dpad_up", "+actionslot 1");
        self NotifyOnPlayerCommand("dpad_down", "+actionslot 2");
        self NotifyOnPlayerCommand("dpad_right", "+actionslot 4");

        self.cj_commands_initialized = true;
    }

    for (;;)
    {
        button = self common_scripts\utility::waittill_any_return("dpad_up", "dpad_down", "dpad_right");
        switch (button)
        {
        case "dpad_up":
            self SpawnBot();
            break;
        // TODO: remove once we have added .cfg loading
        case "dpad_down":
            exec("togglerecord");
            break;
        case "dpad_right":
            exec("startplayback");
            break;
        default:
            iprintln("^1Unknown button " + button);
        }
    }
}

MAX_SAVES = 100;

SavePosition()
{
    if (!self isonground())
        return;

    // Create save struct
    save = spawnstruct();
    save.origin = self.origin;
    save.angles = self GetPlayerAngles();

    // Add to history array
    self.cj.saves[self.cj.saves.size] = save;

    // Limit history size to prevent memory issues
    if (self.cj.saves.size > MAX_SAVES)
    {
        // Remove oldest entry
        new_array = [];
        for (i = 1; i < self.cj.saves.size; i++)
            new_array[new_array.size] = self.cj.saves[i];
        self.cj.saves = new_array;
    }

    self IPrintLn("Position saved (#" + self.cj.saves.size + ")");
}

LoadPosition(savenum)
{
    // Check if position history exists
    if (!IsDefined(self.cj.saves) || self.cj.saves.size == 0)
    {
        self IPrintLn("^1Error: No saved positions found");
        return;
    }

    savenum_provided = IsDefined(savenum);

    // Default to latest position if no specific number provided
    if (!IsDefined(savenum))
        savenum = self.cj.saves.size;

    // Bounds checking
    if (savenum < 1 || savenum > self.cj.saves.size)
    {
        self IPrintLn("^1Error: Invalid position number. Valid range: 1-" + self.cj.saves.size);
        return;
    }

    // Load the specified position (convert to 0-based index)
    save = self.cj.saves[savenum - 1];

    if (!IsDefined(save))
    {
        self IPrintLn("^1Error: Position data corrupted");
        return;
    }

    // Apply the saved position
    self SetVelocity((0, 0, 0));
    self setplayerangles(save.angles);
    self setorigin(save.origin);

    if (self.cj.rpg_switched)
    {
        self switchToWeapon("rpg_mp");
        self.cj.rpg_switched = false;
    }

    // If savenum_provided, print the position number
    if (savenum_provided)
        self IPrintLn("Position loaded (#" + savenum + ")");
}

SpawnBot()
{
    origin = self.origin;
    playerAngles = self GetPlayerAngles();

    if (!IsDefined(self.cj.bot))
    {
        bot = AddTestClient();
        if (!IsDefined(bot))
        {
            self iprintln("Could not add bot");
            wait 1;
            return;
        }

        bot.pers["isBot"] = true;

        bot thread TestClient("axis");
        wait 0.10;
        self.cj.bot = bot;
    }

    wait 0.5;
    for (i = 3; i > 0; i--)
    {
        self iPrintLn("Bot updates in ^2" + i);
        wait 1;
    }
    self.cj.bot SetOrigin(origin);
    // Only set the yaw angle
    self.cj.bot SetPlayerAngles((0, playerAngles[1], 0));
}

TestClient(team)
{
    self endon("disconnect");

    while (!IsDefined(self.pers["team"]))
        wait.05;

    self notify("menuresponse", game["menu_team"], team);
    wait 0.5;

    while (1)
    {
        self notify("menuresponse", "changeclass", "class1");
        self waittill("spawned_player");
        wait(0.10);
    }

    self FreezeControls(true);
}
