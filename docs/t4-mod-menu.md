# CoD Xe Mod Menu: World at War Singleplayer

A GSC mod menu for **Call of Duty: World at War (T4) singleplayer** on Xbox 360: the campaign and
Nazi Zombies, including split-screen and online co-op. It lives in
[`resources/t4/_codxe/mods/mod_menu`](/resources/t4/_codxe/mods/mod_menu). The whole menu is plain
GSC, loaded at runtime by CoD Xe's GSC loader.

## Install

1. Copy `resources/t4/_codxe` into your game directory, as for any CoD Xe mod.
2. Set the active mod in `_codxe/codxe.json`:

   ```json
   {
     "active_mod": "mod_menu"
   }
   ```

3. Start any campaign mission or zombies map. After a few seconds the host sees
   `CoD Xe Menu loaded`.

## Controls

| Action              | Buttons                                             |
| ------------------- | --------------------------------------------------- |
| Open / close        | Hold **LT** and press **RS** (knife)                |
| Move up / down      | **LT** / **RT**, left stick, or D-pad (host)        |
| Select              | **A** (or **X**)                                    |
| Change a value      | **LB** / **RB** or left stick left / right          |
| Back                | **RS**. Pressing it on the main menu closes it      |

Controls are frozen while the menu is open. Toggles show `ON`/`OFF`, sliders show their number, and
choices show the current option.

Only the host gets the menu by default. Use **Players → (player) → Toggle Menu Access** or
**Menu Settings → All Players Get Menu** to share it.

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

Menu Settings: 8 color themes, left/right placement, **Reset All Mods** and About.

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
- locals that are read but never assigned.

Builtin names come from `t4_sp_index.json`. You can regenerate it from your own game with
`build_index.py`, using a dump written by `"dump_rawfile": true` in `codxe.json`.
