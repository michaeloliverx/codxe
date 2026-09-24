# CoD Xe Mod Menu: World at War Singleplayer

A GSC mod menu for **Call of Duty: World at War (T4) singleplayer** on Xbox 360: the campaign and
Nazi Zombies, including split-screen and online co-op. It lives in
[`resources/t4/_codxe/mods/mod_menu`](/resources/t4/_codxe/mods/mod_menu). The whole menu is plain
GSC, loaded at runtime by CoD Xe's GSC loader.

## Setup

### What you need

- Call of Duty: World at War with **Title Update 7** installed. See
  [Installing title updates](title-updates.md).
- CoD Xe running on either:
  - an Xbox 360 that can run unsigned code, or
  - [Xenia Canary](https://github.com/xenia-canary/xenia-canary) with plugins set up as described
    in the [README](../README.md#xenia-canary-setup).
- The game as an **extracted folder**, meaning the folder that contains `default.xex`. CoD Xe reads
  its files from next to the running `default.xex` (`game:` in Xenia), so you can't add them to a
  disc image.

To check that CoD Xe is running, look for the CoD Xe version text drawn on the game's menus.

### Install the menu

1. Copy the `resources/t4/_codxe` folder from this repo (or a release zip) into the game folder, so
   it sits next to `default.xex`:

   ```text
   <game folder>
   |-- default.xex
   `-- _codxe
       |-- codxe.json
       `-- mods
           `-- mod_menu
               `-- maps
                   |-- _music.gsc
                   `-- mod_menu
                       |-- core.gsc
                       |-- menus.gsc
                       `-- ... (the other .gsc files)
   ```

2. Open `_codxe/codxe.json` in a text editor and set the active mod to `mod_menu`. The value must
   match the folder name exactly:

   ```json
   {
     "active_mod": "mod_menu"
   }
   ```

3. Start the game and load any campaign mission or Nazi Zombies map. Once you can move, wait a
   few seconds. Player 1 sees these messages on screen:

   ```text
   CoD Xe Menu loaded
   Hold LT (aim) and press RS (melee) to open the menu
   ```

4. Hold **LT** and click **RS** (press the right stick in) to open the menu.

`codxe.json` picks one mod for both singleplayer and multiplayer. The mod menu only contains
singleplayer scripts, so multiplayer runs unmodded while it is active. To play the `codjumper`
multiplayer mod, set `active_mod` back to `codjumper`.

### Troubleshooting

| Problem | What to check |
| --- | --- |
| No "CoD Xe Menu loaded" message | CoD Xe isn't running (no version text on the menus), or `active_mod` isn't exactly `mod_menu`. Also check that `_codxe/mods/mod_menu/maps/_music.gsc` exists next to `default.xex` and that TU7 is installed. |
| Message shows but the menu won't open | Only player 1 (the host) has the menu by default. Wait until any mission intro has finished. On a different button layout, use your **Aim** and **Melee** buttons (see [Button layouts](#button-layouts)). |
| The level won't load after editing a script | A GSC compile error stops the level from loading. Run the [checker](#validating-gsc-changes) on your changes. |
| Menu feels sluggish | Scripts run on game time, so the menu slows down with **World & Physics → Timescale**. Set it back to 1. |

## Controls

Everything is done with a standard Xbox 360 controller. The table shows the **Default** button
layout.

| Button | Menu closed | Menu open |
| --- | --- | --- |
| **LT** | Hold, then press **RS** to open | Scroll up (hold to repeat) |
| **RT** | | Scroll down (hold to repeat) |
| **A** | | Select, toggle, or apply |
| **X** | | Select (same as A) |
| **RS** (click) | With LT held: open | Back one page. **Hold** to close from anywhere |
| **LB** / **RB** | | Decrease / increase a value, or cycle a choice |
| Left stick | | Up/down scrolls, left/right changes a value |
| D-pad | | Up/down scrolls, left/right changes a value (player 1 only) |
| **B** | Crouch, for the "Crouch + RS" open option | |

While the menu is open you can't move and your gun is lowered, so RT can't fire and the bumpers
can't throw grenades. Closing the menu gives everything back. Opening with LT + RS still plays a
knife swing; that's normal.

The page footer repeats the essentials: `LT/RT Scroll  A Select  RS Back  LB/RB Adjust`.
**Menu Settings → Controls Help** prints the full list in-game.

### Opening options

**Menu Settings → Open With** switches between:

- **LT + RS** (default): hold aim and click melee.
- **Crouch + RS**: press B to crouch, then click melee. Pick this if you often knife while aiming
  and the menu keeps opening in fights.

The choice, theme and menu side are saved per player until you quit the game, so they carry over
between missions.

### Button layouts

The menu reads game actions (aim, fire, jump, use, melee, grenades, movement), not physical
buttons. On a different **Button Layout** or **Stick Layout** in the game's controller options,
use the button that does that action. For example:

- **Tactical** moves melee to **B**, so the menu opens with LT + B, and B is back.
- **Southpaw** swaps the sticks, so scroll with whichever stick moves you.

### D-pad

The D-pad uses the stock `buttonPressed()` script function. It only reads player 1's controller,
and some builds may only allow it in developer mode. If it doesn't respond, everything else
still works.

### Co-op

Each split-screen or online player uses their own controller and has their own menu, cursor and
theme. Only player 1 gets the menu by default. Share it with
**Players → (player) → Toggle Menu Access**, or give everyone access with
**Menu Settings → All Players Get Menu**.

## Features

### Player

God Mode, Demigod (you still feel hits but can't die), Noclip, UFO, Invisible (AI ignore you and
you're hidden), Infinite Ammo (no reloads), Move Speed (0.5x–4x), Super Jump, No Fall Damage,
Unlimited Sprint, Third Person, Field of View (65–120) and Max Health.

Teleport: to your crosshair, save/load position, behind a random enemy, or **Skydive** to the top
of the sky with fall damage off until you land.

### Weapons

- **Give Weapon** lists every weapon the level has actually loaded: your loadout, the level
  loadout, zombie box weapons and weapons the AI are carrying. It never offers one that would fail.
- Random weapon, refill ammo, take current weapon.
- Rapid Fire, Fast Reload, Cluster Grenades, engine-level infinite ammo.
- **Pack-a-Punch** (zombies): upgrades the current weapon when the map has an `_upgraded` version.

### Bullets & Aim

Bullet Mode applies to every shot:

| Mode           | What happens where the bullet lands                                 |
| -------------- | ------------------------------------------------------------------- |
| Explosive      | Explosion with physics push. It never hurts players                 |
| Teleport       | You teleport to the impact point                                    |
| Gib Blaster    | Limbs fly off the target                                            |
| Ragdoll Cannon | The target is killed and launched as a ragdoll                      |
| Tesla Chain    | Electrocutes the target and arcs through up to 6 nearby enemies     |
| Magic Missile  | Fires a real rocket from any rocket weapon the map has loaded       |
| Airstrike      | Six explosions scattered around the impact                          |
| Black Hole     | 6-second vortex that pulls enemies and physics objects in, then detonates |
| Portal Gun     | Shots alternate orange/blue portals; players and AI walk through    |
| Prop Cannon    | Fires random props from the map                                     |
| AI Summoner    | Spawns a soldier (or zombie) where you shoot                        |
| FX Gun         | Plays the effect last picked in the FX Browser                      |

Also Aimbot (hold LT to snap to the nearest visible enemy) and Death Stare (enemies you look at
die).

### Fun & Chaos

- **Chaos Mode**: a random event every 5–60 seconds (moon gravity, bullet time, fast forward, weird
  vision, meteor shower, rapture, mass gibbing, earthquake, blood rain, disco, weapon roulette,
  sonic speed, silent film, hurricane, upside-down physics, enemies launched). Timed events undo
  themselves.
- **Jetpack**: hold A in the air to fly where you look.
- **Rocket Ride**: ride a missile, steer it with your view, and explode on impact.
- **Human Cannonball**, **Ground Pound** (RS in the air slams down with a shockwave) and
  **Force Field**.
- **Matrix Mode** slows time while you aim. Also **Drunk Mode** and **Disco Mode**.
- Tactical Nuke, Meteor Shower, The Rapture (every enemy floats up and explodes), Blood Rain and
  Earthquake.
- **FX Browser**: lists every effect the current level loaded. Selecting one plays it at your
  crosshair and loads it into the FX Gun.

### Enemies

Kill / gib / launch all enemies, bring every enemy to your crosshair, Freeze, One Hit Kills,
Stormtrooper Aim and Enemy Speed. Campaign only: Switch Sides, Civil War (half the enemies turn on
the other half), Pacifist, Invincible Squad and Spawn Bodyguard.

**Spawn Enemy** lists one entry per actor type the level's spawners can create and spawns it at
your crosshair.

### Nazi Zombies

These items only appear on zombies maps.

- Points (+1k / +10k / +100k, reset, infinite).
- All perks or individual perks. They are lost when you go down, like bought perks.
- **Open All Doors**: hold X. Every door and debris pile opens for free.
- **Turn On Power** on maps with a power switch.
- **Drop Power-Up Now** kills the nearest zombie at your feet so the drop lands there. **Rig Next
  Drop** makes the next kill drop your choice.
- Skip Round, Jump To Round (1–100; press A to apply).
- Zombie Speed (walkers / runners / sprinters), **Crawler Army** (every zombie loses its legs),
  **Headless Horde**, Revive Everyone and an on-screen Zombie Counter.

### World & Physics

Timescale (0.1x–3x), Gravity, Ragdoll Gravity (Moon / Zero-G / Upside Down) and Hurricane Winds.
Also detonate every destructible, destroy all vehicles, and **Drivable Vehicles** (walk up to a
tank or truck and press X).

### Visuals

- Vision sets and fog presets (Blood Moon, Toxic, Silent Hill, Midnight, Bubblegum, Crystal Clear).
- Sun color.
- WaW's built-in special features: Black & White, Photo Negative, Super Contrast, **Silent Film**
  (the hidden Chaplin mode), Slow-mo Ability.
- Fullbright, Motion Blur, Double Vision, Film Grain, Hide HUD, Hide Gun.

### Forge

- **Spawn Model** lists props the level placed plus weapon world models, so every entry is loaded.
- **Solid Spawns** uses CoD Xe's `SpawnCollision()`, giving props with collision data real
  collision.
- **Physics Spawns** uses real physics where the model supports it.
- **Grab Mode**: hold LB to pick up the prop you're aiming at, RB spins it, and letting go throws
  it.
- Launch, delete (props, doors, AI), undo and clear.

### Death Cards (campaign)

Toggle the campaign's collectible cheats live, mid-mission: Thunder (explosive headshots that
launch ragdolls), Paintball, Cold Dead Hands, Undead, Hard Headed, Berserker, Vampire, Sticks and
Stones, Flak Jacket, Body Armor, Morphine Shot, Dirty Harry and Hardcore.

### Players & Settings

For each co-op player: bring to you, go to them, god mode, menu access, launch them, revive, and
give points (zombies).

Menu Settings: 8 color themes, left/right placement, the open button combo, Controls Help,
**Reset All Mods** and About.

## How it works

- **Entry point.** `maps/_music.gsc` is the stock file plus one call.
  `_load.gsc` calls `music_init()` on every singleplayer level before the level script's first
  `wait`, so precaching is safe there and the same hook covers campaign and zombies.
- **Works on any map.** Referencing a script that isn't in the level's fastfile is a fatal compile
  error. Campaign levels don't contain the `_zombiemode*` scripts, so the menu never calls them.
  Zombies features drive the same level data and entity triggers the zombie scripts listen to:
  - door triggers, the power switch, `level.zombie_powerup_index`;
  - `level.zombie_total`, `level.scr_anim["zombie"]`.

  Weapon, effect, model and spawner lists are read from the running level.
- **Only guaranteed assets.** Explosions use `level._effect["thunder"]` and gore uses
  `anim._effect["animscript_gib_fx"]`. The stock scripts load both on every singleplayer level.
- **HUD string budget.** Every distinct string passed to `setText()` takes a config string slot
  until the level ends, and overflowing that is the classic mod-menu crash.
  - The list is drawn as one string per page, not one per row.
  - Values use `setValue()` or a small fixed set of strings.
  - Pages are shortened to fit the ~255 character HUD string limit.
- **Script errors are contained.** Each menu action runs in its own thread, so a runtime error only
  ends that action. The input loop has a watchdog that restarts it if it ever stops.
- **CoD Xe builtins used:**
  - the `god`, `noclip` and `ufo` client fields;
  - the `JumpButtonPressed`, `SecondaryOffhandButtonPressed`, `SprintButtonPressed` and
    `Move*ButtonPressed` methods;
  - `SpawnCollision()`.

## Known limitations

- Fog presets can't be undone. Levels set fog once at load and don't store the values, so the
  **Unchanged** option only stops further changes.
- D-pad navigation uses the stock `buttonPressed()`, which may be developer-only on retail builds.
  If it is, the menu quietly falls back to the other buttons.
- Solid Spawns only gives collision to models that have collision data.
- Death Cards appear only in the campaign menu.

## Adding features

Menus are defined in `maps/mod_menu/menus.gsc` with the helpers from `maps/mod_menu/core.gsc`:

```gsc
mm_add_toggle("player", "Moon Boots", "moon_boots", maps\mod_menu\player::moon_boots_set);
mm_add_slider("world", "Gravity", "gravity", maps\mod_menu\world::gravity_set, 50, 1600, 50, 800, "level");
mm_add_action("fun", "Earthquake", maps\mod_menu\fun::earthquake_now);
```

- Toggle callbacks receive `on`.
- Slider callbacks receive the value.
- Choice callbacks receive the index.
- `self` is always the player who used the menu.

## Validating GSC changes

A compile error stops the level from loading, so check scripts offline before copying them to
the console:

```sh
python tools/gsc_check/gsc_check.py resources/t4/_codxe/mods/mod_menu
```

[`tools/gsc_check`](/tools/gsc_check) parses T4 GSC and reports the errors that would stop a level
from loading:
- bad syntax;
- unknown functions or methods;
- builtins called the wrong way;
- scripts that don't exist on every singleplayer map;
- duplicate or colliding functions;
- extra call arguments;
- locals read before they're assigned on every path. This is the compiler's
  `uninitialised variable` error, for example a variable set only inside a loop and read after it;
- `break`/`continue` outside a loop, duplicate `case` values, and assignments to `self`.

The flow rules were checked against about 750 stock scripts. None of the scripts that compile on a
retail build are flagged.

Builtin names come from `t4_sp_index.json`. You can regenerate it from your own game with
`build_index.py`, using a dump written by `"dump_rawfile": true` in `codxe.json`.
