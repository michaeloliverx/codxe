# CoDxe

**CoDxe** is an open-source modding environment for the **Xbox 360** versions of older Call of Duty games.

It supports both physical Xbox 360 consoles and emulation via Xenia.

WHY? As a fun project to explore modding the CoD games I grew up with, and to provide a more flexible and powerful modding experience.

Why the name? CoDxe comes from Call of Duty and Xenon — short, simple, and on brand.

## Requirements

To run CoDxe, you will need one of the following:

- Xbox 360 capable of running unsigned code (JTAG/RGH/DEVKIT/Bad Update)
- [Xenia Canary - Xbox 360 Emulator](https://github.com/xenia-canary/xenia-canary) (Use the latest version)

> [!WARNING]
> Only **Xenia Canary** supports plugins. Master builds will not work.

## Game Compatibility

| Game                                        | Version                                                                                                                                                                   | Singleplayer       | Multiplayer        | Supported Region                                |
| ------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------ | ------------------ | ----------------------------------------------- |
| Call of Duty 2 (2005) (IW2)                 | [TU3](resources/iw2/TU_10LC1UH_0000000000000.0000000000181)                                                                                                               | :white_check_mark: | :white_check_mark: | `Call of Duty 2 (USA, Europe)`                  |
| Call of Duty 4: Modern Warfare (2007) (IW3) | [TU4](resources/iw3/TU_10LC1V6_0000004000000.0000000000101)                                                                                                               | :white_check_mark: | :white_check_mark: | `Call of Duty 4 - Modern Warfare (USA, Europe)` |
| 007: Quantum of Solace (2008)               | [TU2](resources/qos/TU_10LC1VV_000000S000000.00000000000G7)                                                                                                               | :white_check_mark: | :white_check_mark: | `007 - Quantum of Solace (USA, Europe) (En,Fr)` |
| Call of Duty 5: World at War (2008) (T4)    | [TU7](resources/t4/TU_10LC20S_0000010000000.00000000001O8)                                                                                                                | :white_check_mark: | :white_check_mark: | `Call of Duty - World at War (USA, Europe)`     |
| Call of Duty: Modern Warfare 2 (2009) (IW4) | SP [TU9](/resources/iw4/Title%20Updates/TU9/TU_10LC20N_0000018000000.000000000028A) / MP [TU6](/resources/iw4/Title%20Updates/TU6/TU_10LC20N_0000018000000.00000000001GA) | :white_check_mark: | :x:                | `Call of Duty - Modern Warfare 2 (USA, Europe)` |
| Call of Duty: Black Ops (2010) (T5)         | [TU11](/resources/t5/tu00000003_00000000)                                                                                                                                 | :white_check_mark: | :white_check_mark: | `Call of Duty - Black Ops (USA, Europe)`        |
| Call of Duty: Modern Warfare 3 (2011) (IW5) | [TU24](/resources/iw5/tu00000002_00000000)                                                                                                                                | :x:                | :white_check_mark: | `Call of Duty - Modern Warfare 3 (USA Europe)`  |

## Feature Matrix

| Feature                   | IW2 SP             | IW2 MP             | IW3 SP             | IW3 MP             | T4 SP              | T4 MP              | IW4 SP             |
| ------------------------- | ------------------ | ------------------ | ------------------ | ------------------ | ------------------ | ------------------ | ------------------ |
| Raw `.gsc` loader         | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Raw map ents loader       | :x:                | :x:                | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Raw `.dds` texture loader | :x:                | :x:                | :x:                | :white_check_mark: | :x:                | :x:                | :x:                |

## Call of Duty 4 - Modern Warfare

The following is an incomplete version of all the features the client has to offer.

Client Commands:

- `god`
- `noclip`
- `ufo`
- `togglerecord` - start/stop movement recorder
- `startplayback` - start a playback of the current movement recorder
- `imagedump` - Dump all currently loaded textures to disk.

Dvars:

- `pm_pc_mp_velocity_snap` - Enable PC Multiplayer style velocity snapping (round to nearest).
- `pm_multi_bounce` - Enable multi-bounces.
- `noclip_brushes` - Disable player collision for clipmap brush indices e.g. `"50 123 647"`
  - `""` - Empty string restores original collision flag for all brushes.
  - `"*"` disable collision for all brushes.

GSC Entity fields:

- `self.noclip = <bool>` - toggles noclip
- `self.ufo = <bool>` - toggles ufomode
- `self.entityflags = <int>` - gentity flags e.g. `1` is godmode
- `self.forwardmove <int>` - **Read-Only** Player's forward/backward movement input (-127 = full backward, 127 = full forward, 0 = no input)
- `self.rightmove <int>` - **Read-Only** Player's left/right movement input (-127 = full left, 127 = full right, 0 = no input)

GSC Functions:

- `exec` Executes the given command on server as console command e.g. `exec("fast_restart");`

GSC Methods:

- `ButtonPressed` - **HOST ONLY** - Check if a specific button is pressed.
  - Usage: `self ButtonPressed("DPAD_DOWN")`
  - See **keynames** below for valid button identifiers.
- `SprintBreathButtonPressed` - Check if the sprint button is pressed.
- `LeanLeftButtonPressed` - Check if the lean left button is pressed.
- `LeanRightButtonPressed` - Check if the lean right button is pressed.
- `JumpButtonPressed` - Check if the jump button is pressed.
- `HoldBreathButtonPressed` - Check if the hold breath button is pressed.
- `NightVisionButtonPressed` - Check if the night vision button is pressed.
- `ForwardButtonPressed` - Check if the player is moving forward (left stick up).
- `BackButtonPressed` - Check if the player is moving backward (left stick down).
- `LeftButtonPressed` - Check if the player is moving left (left stick left).
- `RightButtonPressed` - Check if the player is moving right (left stick right).
- `SetVelocity` - Changes current player velocity.

  - Usage: `self setVelocity((0, 0, 300)); // Go up.`

- `CloneBrushModelToScriptModel` - Clones a brush model's geometry to a script model entity.
  - Usage: `scriptModel CloneBrushModelToScriptModel(brushEntity);`
- `SetBrushModel` - Sets an entity's brush model by index.
  - Usage: `entity SetBrushModel(index);`

<details>
<summary>keynames</summary>

```
BUTTON_A
BUTTON_B
BUTTON_X
BUTTON_Y
BUTTON_LSHLDR
BUTTON_RSHLDR
BUTTON_START
BUTTON_BACK
BUTTON_LSTICK
BUTTON_RSTICK
BUTTON_RTRIG
BUTTON_LTRIG
DPAD_UP
DPAD_DOWN
DPAD_LEFT
DPAD_RIGHT
APAD_UP
APAD_DOWN
APAD_LEFT
APAD_RIGHT
```

</details>

### Loading single-player maps

Loading single player maps in multiplayer is a best effort approach. Lots of things are broken such as missing FX, player models, crashes etc.

| Name                | Xbox 360 | Xenia |
| ------------------- | :------: | :---: |
| ac130.ff            |    🟢    |  🟢   |
| aftermath.ff        |    🟢    |  🟢   |
| airlift.ff          |    ❌    |  🟢   |
| airplane.ff         |    🟢    |  🟢   |
| ambush.ff           |    ❌    |  🟢   |
| armada.ff           |    ❌    |  ❌   |
| blackout.ff         |    ❌    |  🟢   |
| bog_a.ff            |    ❌    |  🟢   |
| bog_b.ff            |    ❌    |  🟢   |
| cargoship.ff        |    🟢    |  🟢   |
| coup.ff             |    ❌    |  🟢   |
| hunted.ff           |    ❌    |  🟢   |
| icbm.ff             |    ❌    |  🟢   |
| jeepride.ff         |    ❌    |  🟢   |
| killhouse.ff        |    ❌    |  🟢   |
| launchfacility_a.ff |    ❌    |  🟢   |
| launchfacility_b.ff |    ❌    |  🟢   |
| scoutsniper.ff      |    ❌    |  🟢   |
| simplecredits.ff    |    ❌    |  ❌   |
| sniperescape.ff     |    ❌    |  🟢   |
| village_assault.ff  |    ❌    |  🟢   |
| village_defend.ff   |    ❌    |  🟢   |

NOTE: Xenia requires the [patch](<resources/xenia/patches/415607E6 - Call of Duty 4 Modern Warfare MP (TU4).patch.toml>) to increase memory installed.

## Call of Duty: Modern Warfare 2

Loading single player maps in multiplayer is a best effort approach. Lots of things are broken such as missing FX, player models, crashes etc.

🟢 - Loads without issues
🟡 - Texture limit was reached but the out of memory error was patched to not fail. Expect broken and undefined behaviour.
🔴 - Does not load

| Name             | Xbox 360 | Xenia | Notes                                       |
| ---------------- | :------: | :---: | ------------------------------------------- |
| af_caves.ff      |    🟢    |  🟢   |
| af_chase.ff      |    🟢    |  🟢   |
| airport.ff       |    🟡    |  🟢   |
| arcadia.ff       |    🔴    |  🔴   | `Exceeded limit of 1536 'xmodel' assets.`   |
| boneyard.ff      |    🟡    |  🟢   |
| cliffhanger.ff   |    🟡    |  🟢   |
| co_hunted.ff     |    🟢    |  🟢   |
| contingency.ff   |    🟡    |  🟢   |
| dc_whitehouse.ff |    🟢    |  🟢   |
| dcburning.ff     |    🟡    |  🟡   |
| dcemp.ff         |    🟡    |  🟢   |
| ending.ff        |    🔴    |  🔴   | `Exceeded limit of 3584 'image' assets.`    |
| estate.ff        |    🟢    |  🟢   |
| favela.ff        |    🔴    |  🔴   | `Exceeded limit of 4096 'material' assets.` |
| favela_escape.ff |    🔴    |  🔴   | `Exceeded limit of 4096 'material' assets.` |
| gulag.ff         |    🟡    |  🟡   |
| invasion.ff      |    🔴    |  🔴   | `Exceeded limit of 3584 'image' assets.`    |
| oilrig.ff        |    🟡    |  🟢   |
| roadkill.ff      |    🔴    |  🔴   | `Exceeded limit of 4096 'material' assets.` |
| so_bridge.ff     |    🟡    |  🟢   |
| so_ghillies.ff   |    🟢    |  🟢   |
| trainer.ff       |    🟢    |  🟢   |

NOTE: Xenia requires the [patch](<resources\xenia\patches\41560817 - Call of Duty - Modern Warfare 2 MP (TU6).patch.toml>) to increase memory installed.

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
