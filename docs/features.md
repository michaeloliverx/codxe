# Feature Overview

## GSC Loader

The raw GSC loader enables loading `.gsc` scripts directly from the mod directory. This allows you to edit gameplay scripts without modifying or rebuilding the original fastfiles (`.ff`). You can have multiple isolated mods, each with its own set of scripts. The active mod is defined in a `codxe.json` file.

### Setup

Create a `_codxe\mods` folder in your game directory, and place a `codxe.json` in `_codxe` to define which mod is active.

In `codxe.json`, set `"active_mod"` to the name of your mod folder:

```json
{
  "active_mod": "my_mod"
}
```

Once configured, the engine will redirect script lookups to:

```text
game:\_codxe\mods\my_mod\
```

Example tree structure:

```text
game:.
`-- _codxe
    |-- codxe.json
    `-- mods
        `-- my_mod
            `-- maps
                `-- mp
                    `-- gametypes
                        |-- _callbacksetup.gsc
                        `-- custom_logic.gsc
```

The underscore keeps the folder at the top of file listings and separates CoD Xe system files from the game's core content.

### Script Overriding

The loader operates using a virtual filesystem. All `.gsc` scripts, whether loaded from `.ff` files or external mod folders, are treated as if they exist in a single shared root.

For example:

- A file in `common_mp.ff/maps/mp/gametypes/_callbacksetup.gsc`
- Can be overridden by `game:\_codxe\mods\my_mod\maps\mp\gametypes\_callbacksetup.gsc`

Your version will take precedence over the copy embedded in the original fastfile. You can also add new scripts and call them from overridden entry points such as `_callbacksetup.gsc`.

> [!NOTE]
> You must override at least one existing script, such as `_callbacksetup.gsc`, to gain control over the script VM and begin loading your own logic.

## Map Ents Loader

Map entity files (`ents`) define dynamic scriptable entities in a level, including spawn points, game objects, `script_model`s, pickups, and more. CoD Xe allows you to override these definitions with a custom ents file placed inside your mod folder.

Example format:

```text
{
  "gndLt" "2f37473d01"
  "ltOrigin" "-2554.97 4954.71 94.4048"
  "targetname" "tarps"
  "origin" "-2554.5 4942.8 73.9"
  "model" "training_camo_tarp"
  "classname" "script_model"
  "angles" "0 270 0"
}
{
  "gndLt" "36373e0005"
  "ltOrigin" "-3544.2 2426.2 -157"
  "lighttarget" "pf79_auto24"
  "targetname" "pit_case_02"
  "origin" "-3544 2392 -192"
  "angles" "0 270 0"
  "model" "com_plasticcase_beige_big_us_dirt_animated"
  "classname" "script_model"
}
```
