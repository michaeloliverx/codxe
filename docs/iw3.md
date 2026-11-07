# CoD Xe IW3: Call of Duty 4 - Modern Warfare

The following is an incomplete list of features currently exposed by the IW3 client.

## Multiplayer

### Client Commands

- `god`
- `noclip`
- `ufo`
- `togglerecord` - Start or stop the movement recorder.
- `startplayback` - Start playback of the current movement recorder.
- `imagedump` - Dump all currently loaded textures to disk.

### Dvars

- `pm_pc_mp_velocity_snap` - Enables PC Multiplayer style velocity snapping by rounding to nearest.
- `pm_multi_bounce` - Enables multi-bounces.
- `noclip_brushes` - Disables player collision for clipmap brush indices, for example `"50 123 647"`.
- `cm_showCollision` - World and solid brush-entity collision outlines: `0` off (default), `1` invisible player clip barriers, `2` all player collision. Outlines are cyan and draw through world geometry without writing depth.
- `cm_showBounces` - Show bounce candidates: slope highlights on world brushes, terrain, and active solid brush-model entities, plus magenta terrain edge/corner markers. Entity collision follows its current position and rotation; slope angles are tested in world space. Defaults to `0`; set to `1` to enable independently of `cm_showCollision`. Slope colors run from green to red over the same normal-Z range (`0.3` to `0.7`) used by iw3mptool. This face-angle filter covers roughly 45.6 to 72.5 degrees above horizontal. Actual bounces still depend on movement and approach.
- `cm_showBounceEdges` - Optional edge-marker filter within `cm_showBounces`; defaults to `1`. Set to `0` for slope highlights only. Magenta ticks mark terrain edges/corners where a native player-capsule sweep finds a non-walkable edge contact with an upward velocity-clipping response. This samples representative approaches, not a full jump simulation or a guarantee that every edge bounce is found.
- `cm_showBounceEdgeDist` - Edge probe distance in game units. Defaults to `256`; range `64` to `1000`, also limited by `cm_showCollisionDist`. Results fill in over several frames, with at most 32 native sweeps per view per frame. Uses the current prediction bounds when available, otherwise a standing capsule.
- `cm_showCollisionDist` - Maximum outline distance from the current camera in game units. Defaults to `1000`; range `64` to `10000`.
- `cm_showCollisionMaxLines` - Per-view outline budget. Defaults to `1024`; range `64` to `4096`. Nearest collision draws first. Newly encountered brushes fill in over several frames to limit clipping work.
- `r_drawDynEnts` - Draws dynamic entities.
- `bg_bobIdle` - Controls idle gun sway.
- `cg_mirrorWorld` - Experimental horizontal world mirror. Defaults to `false`; set to `1` to enable or `0` to disable live. Keeps the first-person weapon and HUD normal and reverses horizontal look, strafe, and lean controls.
- `cg_scoreboardLabel_Score` - Overrides the label for the `Score` column on the scoreboard.
- `cg_scoreboardLabel_Kills` - Overrides the label for the `Kills` column on the scoreboard.
- `cg_scoreboardLabel_Assists` - Overrides the label for the `Assists` column on the scoreboard.
- `cg_scoreboardLabel_Deaths` - Overrides the label for the `Deaths` column on the scoreboard.

Special `noclip_brushes` values:

- `""` - Restores the original collision flag for all brushes.
- `"*"` - Disables collision for all brushes.

To show barriers and bounce candidates together:

```text
cm_showCollision 1
cm_showBounces 1
```

Set both to `0` to turn the overlay off. Outlines include BSP world geometry and active solid brush-model entities, such as objective props and moving platforms. Engine-managed dynamic brushes are included when linked and enabled for player collision by the engine. Triggers, render-only model meshes, and character collision boxes are not drawn. Magenta edge probes currently cover world terrain only. The separate debug drawing module also restores native client and GSC debug line/text submission in TU4.

### GSC Entity Fields

- `self.noclip = <bool>` - Toggles noclip.
- `self.ufo = <bool>` - Toggles UFO mode.
- `self.entityflags = <int>` - Sets gentity flags. For example, `1` is god mode.
- `self.forwardmove <int>` - Read-only player forward/backward movement input. `-127` is full backward, `127` is full forward, and `0` is no input.
- `self.rightmove <int>` - Read-only player left/right movement input. `-127` is full left, `127` is full right, and `0` is no input.

### GSC Functions

- `exec` - Executes the given command on the server as a console command, for example `exec("fast_restart");`.

### GSC Methods

- `ButtonPressed` - Host-only check for a specific button press. Usage: `self ButtonPressed("DPAD_DOWN")`.
- `SprintBreathButtonPressed` - Checks if the sprint button is pressed.
- `LeanLeftButtonPressed` - Checks if the lean left button is pressed.
- `LeanRightButtonPressed` - Checks if the lean right button is pressed.
- `JumpButtonPressed` - Checks if the jump button is pressed.
- `HoldBreathButtonPressed` - Checks if the hold breath button is pressed.
- `NightVisionButtonPressed` - Checks if the night vision button is pressed.
- `ForwardButtonPressed` - Checks if the player is moving forward.
- `BackButtonPressed` - Checks if the player is moving backward.
- `LeftButtonPressed` - Checks if the player is moving left.
- `RightButtonPressed` - Checks if the player is moving right.
- `SetVelocity` - Changes current player velocity. Usage: `self setVelocity((0, 0, 300));`.
- `SetStance` - Changes current player stance.
- `BotAction` - Same as [CoD4x BotAction](https://github.com/callofduty4x/CoD4x_Server/blob/master/scriptdocumentation/script_functions_reference.md#botentity-botactionsignaction).
- `BotStop` - Same as [CoD4x BotStop](https://github.com/callofduty4x/CoD4x_Server/blob/master/scriptdocumentation/script_functions_reference.md#botentity-botstop).
- `CloneBrushModelToScriptModel` - Clones a brush model's geometry to a script model entity. Usage: `scriptModel CloneBrushModelToScriptModel(brushEntity);`.
- `SetBrushModel` - Sets an entity's brush model by index. Usage: `entity SetBrushModel(index);`.

<details>
<summary>Button key names</summary>

```text
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

## Loading Singleplayer Maps In Multiplayer

Loading singleplayer maps in multiplayer is a best-effort approach. Many things are broken, including missing FX, player models, and crashes.

| Name                  | Xbox 360 |        Xenia        |
| --------------------- | :------: | :-----------------: |
| `ac130.ff`            |  Loads   |        Loads        |
| `aftermath.ff`        |  Loads   |        Loads        |
| `airlift.ff`          |  Fails   |        Loads        |
| `airplane.ff`         |  Loads   |        Loads        |
| `ambush.ff`           |  Fails   |        Fails        |
| `armada.ff`           |  Fails   |        Fails        |
| `blackout.ff`         |  Fails   | Fails: xmodel limit |
| `bog_a.ff`            |  Fails   |        Loads        |
| `bog_b.ff`            |  Fails   | Fails: xmodel limit |
| `cargoship.ff`        |  Loads   |        Loads        |
| `coup.ff`             |  Fails   |        Loads        |
| `hunted.ff`           |  Fails   |        Loads        |
| `icbm.ff`             |  Fails   |        Loads        |
| `jeepride.ff`         |  Fails   |        Loads        |
| `killhouse.ff`        |  Fails   |        Loads        |
| `launchfacility_a.ff` |  Fails   |        Loads        |
| `launchfacility_b.ff` |  Fails   | Fails: xmodel limit |
| `scoutsniper.ff`      |  Fails   |        Loads        |
| `simplecredits.ff`    |  Fails   |        Fails        |
| `sniperescape.ff`     |  Fails   |        Loads        |
| `village_assault.ff`  |  Fails   | Fails: xmodel limit |
| `village_defend.ff`   |  Fails   | Fails: xmodel limit |

> [!NOTE]
> Xenia requires the [IW3 MP TU4 patch](</resources/xenia/patches/415607E6%20-%20Call%20of%20Duty%204%20Modern%20Warfare%20MP%20(TU4).patch.toml>) to increase installed memory.
