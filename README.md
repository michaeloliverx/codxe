# CoDxe

**CoDxe** is an open-source modding environment for the **Xbox 360** versions of older Call of Duty games.

It supports both physical Xbox 360 consoles and emulation via Xenia.

WHY? As a fun project to explore modding the CoD games I grew up with, and to provide a more flexible and powerful modding experience.

## Requirements

To run CoDxe, you will need one of the following:

- Xbox 360 capable of running unsigned code (JTAG/RGH/DEVKIT/Bad Update)
- [Xenia Canary - Xbox 360 Emulator](https://github.com/xenia-canary/xenia-canary) (Use the latest version)

> [!WARNING]
> Only **Xenia Canary** supports plugins. Master builds will not work.

## Game Compatibility

| Game                                        | Version                                                      | Singleplayer       | Multiplayer        | Supported Region                                |
| ------------------------------------------- | ------------------------------------------------------------ | ------------------ | ------------------ | ----------------------------------------------- |
| Call of Duty 2 (2005) (IW2)                 | [TU3](resources/iw2/TU_10LC1UH_0000000000000.0000000000181)  | :white_check_mark: | :x:                | `Call of Duty 2 (USA, Europe)`                  |
| Call of Duty 4: Modern Warfare (2007) (IW3) | [TU4](resources/iw3/TU_10LC1V6_0000004000000.0000000000101)  | :white_check_mark: | :white_check_mark: | `Call of Duty 4 - Modern Warfare (USA, Europe)` |
| Call of Duty 5: World at War (2008) (T4)    | [TU7](resources/t4/TU_10LC20S_0000010000000.00000000001O8)   | :white_check_mark: | :white_check_mark: | `Call of Duty - World at War (USA, Europe)`     |
| Call of Duty: Modern Warfare 2 (2009) (IW4) | [TU9](/resources/iw4/TU_10LC20N_0000018000000.000000000028A) | :white_check_mark: | :x:                | `Call of Duty 4 - Modern Warfare (USA, Europe)` |

## Feature Matrix

| Feature                   | IW2 SP             | IW2 MP             | IW3 SP             | IW3 MP             | T4 SP              | T4 MP              | IW4 SP             |
| ------------------------- | ------------------ | ------------------ | ------------------ | ------------------ | ------------------ | ------------------ | ------------------ |
| Raw `.gsc` loader         | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Raw map ents loader       | :x:                | :x:                | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Raw `.dds` texture loader | :x:                | :x:                | :x:                | :white_check_mark: | :x:                | :x:                | :x:                |

## Features

### GSC Loader

The raw GSC loader enables loading .gsc scripts directly from the mod directory. This allows you to edit gameplay scripts without modifying or rebuilding the original fastfiles (`.ff`). You can have multiple isolated mods, each with its own set of scripts. The active mod is defined in a `codxe.json` file.

#### Setup Instructions

Create a `_codxe\mods` folder in your game directory, and place a `codxe.json` in `_codxe` to define which mod is active.

In `codxe.json`, set `"active_mod"` to the name of your mod folder:

```json
{
  "active_mod": "my_mod"
}
```

Once configured, the engine will redirect script lookups to:

```
game:\_codxe\mods\my_mod\
```

**Example tree structure:**

```
game:.
└───_codxe
    │   codxe.json                                  ← sets the active mod
    │
    └───mods
        └───my_mod                                   ← your mod folder (name must match config)
            └───maps
                └───mp
                    └───gametypes
                            _callbacksetup.gsc       ← overrides built-in script
                            custom_logic.gsc         ← your custom script

```

**Why the underscore in `_codxe`?**

The underscore keeps the folder at the top of file listings, making it quicker to find. It also separates CoDxe system files from the game's core content for better organization.

#### :scroll: Script Overriding

The loader operates using a virtual filesystem. All `.gsc` scripts — whether loaded from `.ff` files or external mod folders — are treated as if they exist in a single shared root.

For example:

- A file in `common_mp.ff/maps/mp/gametypes/_callbacksetup.gsc`
- Can be overridden by a file in `game:\_codxe\mods\my_mod\maps\mp\gametypes\_callbacksetup.gsc`

Your version will take precedence over the copy embedded in the original fastfile. You can also add **new scripts** and call them from overridden entry points (e.g. `_callbacksetup.gsc`) — just ensure you hook them properly.

> [!NOTE]
> You **must override** at least one existing script (like `_callbacksetup.gsc`) to gain control over the script VM and begin loading your own logic.

### Map Ents Loader

**What are map ents?**
Map entity files (`ents`) define the dynamic scriptable entities in a level-spawn points, game objects, `script_model`s, pickups, and more. CoDxe allows you to override these definitions with a custom ents file placed inside your mod folder.

**Example Format**
Each entity is defined using a block of key-value pairs:

```
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

### GSC Extensions

The GSC VM has been extended with additional features to support enhanced modding. Each game has its own set of extensions, see below for details.

#### Client Fields

| Field         | Type   | Description                                                               | Supported in             |
| ------------- | ------ | ------------------------------------------------------------------------- | ------------------------ |
| `self.ufo`    | `bool` | Enables UFO mode, allowing the player to freely fly around the map.       | IW3 MP, T4 SP/MP, IW4 SP |
| `self.noclip` | `bool` | Enables no-clip mode, letting the player pass through walls and geometry. | IW3 MP, T4 SP/MP, IW4 SP |
| `self.god`    | `bool` | Enables God mode, making the player invulnerable to all damage.           | IW3 MP, T4 SP/MP, IW4 SP |

## Getting Started

### Xenia Canary Setup

_Required config settings:_

```toml
# xenia.config.toml
allow_plugins = true
allow_game_relative_writes = true
```

### 🚧 Coming Soon

- Build instructions
- Setup guides for JTAG/RGH/DEVKIT consoles
- Xenia setup guide
- xenia canary netplay
- xenia canary install content (title updates, DLC, etc.)
- Modding tutorials with example scripts
- Template mod structure for each game
- https://consolemods.org/wiki/Xbox_360:Manually_Installing_Title_Updates
- details on xenia Module Hash: , using a different executable etc.

## Credits

In some way or another, this project would not have been possible without the following people and projects:

- [callofduty4x](https://github.com/callofduty4x)
- [IW4x](https://github.com/iw4x)
- [kej](https://github.com/kejjjjj)
- [ClementDreptin](https://github.com/ClementDreptin/ModdingResources)
