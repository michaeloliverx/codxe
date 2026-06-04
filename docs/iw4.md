# CoD Xe IW4: Call of Duty: Modern Warfare 2

IW4 has two supported title update targets:

- **TU9** supports singleplayer and multiplayer.
- **MP TU6** supports multiplayer only, with additional features.

## TU9

### Singleplayer

Supported features include:

- Raw `.gsc` loader.
- Raw map ents loader.
- GSC function extension support.
- Added client fields:
  - `self.clientflags`
  - `self.entityflags`
- Movement patches.
- Debug/UI support.

#### GSC Functions

##### `exec(string <command>)`

Executes the given command as a console command.

Usage example:

```gsc
exec("map trainer");
```

#### GSC Client Fields

##### `clientflags` _(int)_

Exposes the player's client flags.

##### `entityflags` _(int)_

Exposes the backing entity flags. This can be used for flag-based behavior such as god mode.

Usage example:

```gsc
self.entityflags = 1;
```

### Multiplayer

Supported features include:

- Disable auth requirement for fastfiles
- Re-enables CoD4-style bouncing

#### Dvars

##### `bg_rocketJump` _(bool)_

Enables CoD4-style rocket jumps.

##### `bg_rocketJumpScale` _(float)_

Controls the pushback scale applied by rocket jumps.

## MP TU6

MP TU6 is a multiplayer-only target. It exists separately from TU9 because some IW4 multiplayer work targets TU6 specifically.

Supported features include:

- Raw `.gsc` loader.
- Raw map ents loader.
- GSC function and method extension support.
- Added client fields:
  - `self.clientflags`
  - `self.entityflags`
- Developer console.
- Movement recorder.
- Singleplayer-map-in-multiplayer support.
- Playerclip brush collision controls.

### GSC Functions

#### `exec(string <command>)`

Executes the given command as a console command.

Usage example:

```gsc
exec("fast_restart");
```

### Dvars

#### `noclip_brushes` _(string)_

Controls which playerclip brushes have collision disabled.

Special values:

- `""` - Restores all brushes to their original collision state.
- `"*"` - Disables player collision on every brush in the clipmap.

Usage examples:

```text
set noclip_brushes ""
set noclip_brushes "*"
set noclip_brushes "4 7"
```

### GSC Player Methods

#### `DisablePlayerClipOnIntersectingBrushes()`

Finds playerclip brushes intersecting the player's bounds and appends them to `noclip_brushes`.

Usage example:

```gsc
self DisablePlayerClipOnIntersectingBrushes();
```

### GSC Client Fields

#### `clientflags` _(int)_

Exposes the player's client flags.

#### `entityflags` _(int)_

Exposes the backing entity flags.

Usage example:

```gsc
self.entityflags = 1;
```

## MP TU6: Loading Singleplayer Maps In Multiplayer

Loading singleplayer maps in multiplayer is a best-effort approach. Many things are broken, including missing FX, player models, and crashes.

Status key:

- `Loads` - Loads without known major issues.
- `Limited` - Texture limit was reached, but the out-of-memory error was patched to not fail. Expect broken or undefined behavior.
- `Fails` - Does not load.

| Name               | Xbox 360 |  Xenia  | Notes                                       |
| ------------------ | :------: | :-----: | ------------------------------------------- |
| `af_caves.ff`      |  Loads   |  Loads  |                                             |
| `af_chase.ff`      |  Loads   |  Loads  |                                             |
| `airport.ff`       | Limited  |  Loads  |                                             |
| `arcadia.ff`       |  Fails   |  Fails  | `Exceeded limit of 1536 'xmodel' assets.`   |
| `boneyard.ff`      | Limited  |  Loads  |                                             |
| `cliffhanger.ff`   | Limited  |  Loads  |                                             |
| `co_hunted.ff`     |  Loads   |  Loads  |                                             |
| `contingency.ff`   | Limited  |  Loads  |                                             |
| `dc_whitehouse.ff` |  Loads   |  Loads  |                                             |
| `dcburning.ff`     | Limited  | Limited |                                             |
| `dcemp.ff`         | Limited  |  Loads  |                                             |
| `ending.ff`        |  Fails   |  Fails  | `Exceeded limit of 3584 'image' assets.`    |
| `estate.ff`        |  Loads   |  Loads  |                                             |
| `favela.ff`        |  Fails   |  Fails  | `Exceeded limit of 4096 'material' assets.` |
| `favela_escape.ff` |  Fails   |  Fails  | `Exceeded limit of 4096 'material' assets.` |
| `gulag.ff`         | Limited  | Limited |                                             |
| `invasion.ff`      |  Fails   |  Fails  | `Exceeded limit of 3584 'image' assets.`    |
| `oilrig.ff`        | Limited  |  Loads  |                                             |
| `roadkill.ff`      |  Fails   |  Fails  | `Exceeded limit of 4096 'material' assets.` |
| `so_bridge.ff`     | Limited  |  Loads  |                                             |
| `so_ghillies.ff`   |  Loads   |  Loads  |                                             |
| `trainer.ff`       |  Loads   |  Loads  |                                             |

> [!NOTE]
> Xenia requires the [IW4 MP TU6 patch](</resources/xenia/patches/41560817%20-%20Call%20of%20Duty%20-%20Modern%20Warfare%202%20MP%20(TU6).patch.toml>) to increase installed memory.
