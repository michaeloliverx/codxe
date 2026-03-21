# CoDxe IW2: Call of Duty 2

## Multiplayer

### General Functions

#### `exec(string <command>)`

Executes the given command on server as console command.

Usage example: `exec("fast_restart");`

### Player Methods

#### `ButtonPressed(string <button>)`

Returns true if the specified button is currently pressed. Only works for the host player.

Valid button names:

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

Usage example: `if (self ButtonPressed("BUTTON_A")) { ... }`

<details>
<summary>Full example</summary>

```gsc
WatchButtons()
{
    self endon("disconnect");
    self endon("end_respawn");
    self endon("death");

    keynames = [];
    keynames[keynames.size] = "BUTTON_A";
    keynames[keynames.size] = "BUTTON_B";
    keynames[keynames.size] = "BUTTON_X";
    keynames[keynames.size] = "BUTTON_Y";
    keynames[keynames.size] = "BUTTON_LSHLDR";
    keynames[keynames.size] = "BUTTON_RSHLDR";
    keynames[keynames.size] = "BUTTON_START";
    keynames[keynames.size] = "BUTTON_BACK";
    keynames[keynames.size] = "BUTTON_LSTICK";
    keynames[keynames.size] = "BUTTON_RSTICK";
    keynames[keynames.size] = "BUTTON_RTRIG";
    keynames[keynames.size] = "BUTTON_LTRIG";
    keynames[keynames.size] = "DPAD_UP";
    keynames[keynames.size] = "DPAD_DOWN";
    keynames[keynames.size] = "DPAD_LEFT";
    keynames[keynames.size] = "DPAD_RIGHT";
    keynames[keynames.size] = "APAD_UP";
    keynames[keynames.size] = "APAD_DOWN";
    keynames[keynames.size] = "APAD_LEFT";
    keynames[keynames.size] = "APAD_RIGHT";

    for (;;)
    {
        for(i=0;i<keynames.size;i++)
        {
            if (self ButtonPressed(keynames[i]))
            {
                self iprintln(keynames[i] + " Pressed!");
                wait 0.2;
            }
        }

        wait 0.05;
    }
}
```

</details>

#### `ADSButtonPressed()`

Usage example: `if (self ADSButtonPressed()) { ... }`

#### `JumpButtonPressed()`

Usage example: `if (self JumpButtonPressed()) { ... }`

#### `FragButtonPressed()`

Usage example: `if (self FragButtonPressed()) { ... }`

#### `SmokeButtonPressed()`

Usage example: `if (self SmokeButtonPressed()) { ... }`

#### `ForwardButtonPressed()`

Returns true if the player is moving forward.

Usage example: `if (self ForwardButtonPressed()) { ... }`

#### `BackButtonPressed()`

Returns true if the player is moving back.

Usage example: `if (self BackButtonPressed()) { ... }`

#### `LeftButtonPressed()`

Returns true if the player is moving left.

Usage example: `if (self LeftButtonPressed()) { ... }`

#### `RightButtonPressed()`

Returns true if the player is moving right.

Usage example: `if (self RightButtonPressed()) { ... }`

#### `GetStance()`

Returns the player's current stance as a string: `"stand"`, `"crouch"`, or `"prone"`.

Usage example: `if (self GetStance() == "crouch") { ... }`

#### `SetStance(string <stance>)`

Sets the player's stance. Valid values are `"stand"`, `"crouch"`, or `"prone"`.

Usage example: `self SetStance("prone");`

#### `GetVelocity()`

Returns the player's current velocity as a 3D vector `(x, y, z)`.

Usage example: `vel = self GetVelocity();`

#### `SetVelocity(vec3 <velocity>)`

Sets the player's velocity to the given 3D vector.

Usage example: `self SetVelocity((0, 0, 300)); // go up`
