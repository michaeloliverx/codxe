# Feature Overview

## GSC Loader

The raw GSC loader enables loading `.gsc` scripts directly from the mod directory. This allows you to edit gameplay scripts without modifying or rebuilding the original fastfiles (`.ff`). You can have multiple isolated mods, each with its own set of scripts. The active mod is defined in a `codxe.json` file.

### Setup

Create a `_codxe\<gameID>\mods` folder in your game directory, and place a `codxe.json` in `_codxe\<gameID>` to define which mod is active. Use `iw2`, `iw3`, `iw4`, `iw5`, `t4`, `t5`, or `qos` for the game ID.

The repository and release archive contain one top-level [`_codxe`](/_codxe) folder with a subdirectory for each game. Copy that folder into your game directory, preserving `_codxe\<gameID>\...`. T4 singleplayer also supports copying the same folder to a USB or HDD root. Title updates and Xenia files remain separate from the game data.

CoD Xe selects one data root at startup. If `_codxe\<gameID>` exists, all config, mod, fastfile, usermap, internal asset, dump, and player stats paths use that root. Missing files never fall back to the legacy `_codxe` root. The old layout remains supported when the nested directory does not exist. Keep all data for a game together when migrating.

When neither local directory exists, new data uses `_codxe\<gameID>`. T4 singleplayer retains its USB/HDD lookup and requires an existing data root; see the [T4 guide](t4.md).

In `codxe.json`, set `"active_mod"` to the name of your mod folder:

```json
{
  "active_mod": "my_mod"
}
```

Once configured, the engine will redirect script lookups to:

```text
game:\_codxe\<gameID>\mods\my_mod\
```

Example tree structure:

```text
game:.
`-- _codxe
    `-- <gameID>
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
- Can be overridden by `game:\_codxe\<gameID>\mods\my_mod\maps\mp\gametypes\_callbacksetup.gsc`

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
