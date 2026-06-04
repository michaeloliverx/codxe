# CoD Xe 007: Quantum of Solace

## Multiplayer

### DVARs

#### `noclip_brushes` _(string)_

A server-info (synced to all clients) dvar that controls which brushes have their `PLAYERCLIP` flag disabled. It holds a space-separated list of brush indices.

Special values:

- `""` (empty) - restores all brushes to their original collision state
- `"*"` - disables `PLAYERCLIP` flag on every brush in the clipmap

```
set noclip_brushes "";    // reset all brush collision
set noclip_brushes "*";   // disable all brush collision
set noclip_brushes "4 7"; // disable collision on brushes 4 and 7
```

### GSC Functions

#### `exec(string <command>)`

Executes the given command on server as console command.

Usage example: `exec("fast_restart");`

### GSC Player Methods

#### `ButtonPressed(string <button>)`

A host-only method to check if any button is pressed by key name.

Usage example: `self ButtonPressed("DPAD_DOWN") { ... }`

#### `ADSButtonPressed()`

Usage example: `if (self ADSButtonPressed()) { ... }`

#### `JumpButtonPressed()`

Usage example: `if (self JumpButtonPressed()) { ... }`

#### `NextFireTypeButtonPressed()`

On console this is bound to DPAD_UP and is used to control auto vs single shot guns.

Usage example: `if (self NextFireTypeButtonPressed()) { ... }`

#### `SprintButtonPressed()`

Usage example: `if (self SprintButtonPressed()) { ... }`

#### `SetVelocity(<vec velocity>)`

Changes current player velocity.

Usage example: `self SetVelocity((0, 0, 300)); // Go up.`

#### `DisablePlayerClipOnTouchingBrushes()`

Disables `PLAYERCLIP` collision on all brushes whose AABB touches or overlaps the player's bounding box. This includes brushes the player is standing on top of, not just ones they are inside.

The affected brush indices are written to the `noclip_brushes` dvar so the state persists and can be restored.

Usage example:

```
self DisablePlayerClipOnTouchingBrushes();
```

### GSC Client Fields

#### `noclip` _(bool)_

Enables noclip on the player - they can fly freely and pass through walls.

Usage example:

```
self.noclip = true;         // noclip on
self.noclip = false;        // noclip off
self.noclip = !self.noclip; // noclip toggle
```

#### `ufo` _(bool)_

Enables UFO mode on the player - they can fly freely and pass through walls.

Usage example:

```
self.ufo = true;        // ufo on
self.ufo = false;       // ufo off
self.ufo = !self.ufo;   // ufo toggle
```

#### `entityflags` _(int, bitflag)_

A bitmask field on the player entity. Individual flags must be toggled with bitwise operators - assigning directly will overwrite all other active flags.

```
self.entityflags |= 0x1;  // set a flag
self.entityflags &= ~0x1; // clear a flag
```

Known values (there are more):

```
FL_GODMODE      = 0x1,
FL_DEMI_GODMODE = 0x2,
```
