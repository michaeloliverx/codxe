# CoDxe

## Overview

**CoDxe** is an open-source modding environment for the **Xbox 360** versions of older Call of Duty games.

It supports both physical Xbox 360 consoles and emulation via Xenia.

### Requirements

To run CoDxe, you will need one of the following:

- Xbox 360 capable of running unsigned code (JTAG/RGH/DEVKIT/Bad Update)
- [Xenia Canary - Xbox 360 Emulator](https://github.com/xenia-canary/xenia-canary) (Use the latest version)

## Game Compatibility

| Game                                        | Version | Singleplayer       | Multiplayer        | Supported Region                                |
| ------------------------------------------- | ------- | ------------------ | ------------------ | ----------------------------------------------- |
| Call of Duty 4: Modern Warfare (2007) (IW3) | TU4     | :white_check_mark: | :white_check_mark: | `Call of Duty 4 - Modern Warfare (USA, Europe)` |
| Call of Duty 5: World at War (2008) (T4)    | TU7     | :white_check_mark: | :white_check_mark: | `Call of Duty - World at War (USA, Europe)`     |
| Call of Duty: Modern Warfare 2 (2009) (IW4) | TU9     | :white_check_mark: | :x:                | `Call of Duty 4 - Modern Warfare (USA, Europe)` |

## Feature Matrix

| Feature                   | IW3 SP             | IW3 MP             | T4 SP              | T4 MP              | IW4 SP             |
| ------------------------- | ------------------ | ------------------ | ------------------ | ------------------ | ------------------ |
| Raw `.gsc` loader         | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Raw map ents loader       | :white_check_mark: | :white_check_mark: | :x:                | :white_check_mark: | :white_check_mark: |
| Raw `.dds` texture loader | :x:                | :white_check_mark: | :x:                | :x:                | :x:                |

## Features

### GSC Loader

The raw GSC loader enables loading .gsc scripts directly from the mod directory. This allows you to edit gameplay scripts without modifying the original fastfiles. See the example mods for guidance on structure and usage.

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

#### IW4 SP

**New Built-in player fields**

- `self.ufo` – Enables UFO mode, allowing the player to freely fly around the map.
- `self.noclip` – Enables no-clip mode, letting the player pass through walls and geometry.
- `self.god` – Enables God mode, making the player invulnerable to all damage.

## Getting Started

Coming soon: Build instructions, example mods, and modding tutorials.
