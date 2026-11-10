# Call of Duty: Modern Warfare - Codjumper Mod

## Controls

### CJ Controls
- **Save position**: *Double RS button*
- **Load position**: *Press LB button*
- **Load previous position(s)**: *HOLD LB button and press X to cycle through saved positions*
- **UFO on/off**: *RB*
- **Spawn static bot**: DPAD Up
- **Air-strafe assist**: Hold DPAD Right while moving in the air. Adjusts yaw using your current stick direction; release to resume manual yaw control. Pitch and movement input are preserved.
- **Optional strafe guide**: Set `cj_strafe_guide 1` to show the target band and yaw correction. Defaults to off and works without holding the assist button.
- **Target strafe mode**: Aim at a world surface and run `cj_strafe_settarget`, then set `cj_strafe_mode 2`. Jump and hold DPAD Right to let the helper steer yaw and movement toward that point. Release to regain control. Pitch, jump and RPG timing remain manual. Use `cj_strafe_mode 1` to return to stick assistance, or `cj_strafe_cleartarget` to remove the target.
- **Turn speed**: `cj_strafe_yaw_speed` defaults to `180` degrees per second and limits view turning in both modes. Target mode predicts clear-air movement; it does not avoid obstacles or guarantee a landing/bounce.
- **Assist diagnostics**: Marking a target restores the DPAD Right helper binding for the current controller layout. The target HUD shows the actual activation state. Run `cj_strafe_status` to show the live binding and write a status snapshot to the Xenia/debug log; button edges and activation changes are also logged automatically.

### Menu Controls
- **Open menu**: *Double X*
- **Close menu**: *RS*
- **Scroll up/down**: *LT/RT*
- **Select option**: *X*

### Forge Controls
- **Open forge**: *While in UFO mode, hold X and press RS*
- **Clone object**: *While in forge, hold X and press LS*
  - This will clone the object you are looking at.
  - **Drop cloned object**: *Hold X and press RT*

## Movement Recorder
- **Start/Stop recording**: *DPAD Down*
- **Play recorded movement**: Run `startplayback` from the console or bind it to another button.
