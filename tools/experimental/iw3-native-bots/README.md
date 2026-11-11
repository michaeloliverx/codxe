# IW3 Xbox 360 native controller experiment - diagnostics 6

Diagnostics 6 fixes a client-capacity overflow in the native controller. See [CRASH-91D5AF64.md](CRASH-91D5AF64.md) for the address trace and regression evidence. If the diagnostics 5 mod scripts are already installed, this fix only requires replacing the XEX and restarting IW3; no script/config changes are required.

This build replaces the active Bot Warfare combat loop with a small native C++ controller. It deliberately changes behaviour to measure how much performance can be recovered at 17 bots. Console performance and gameplay still need a hardware retest.

Scripts retain bot management, teams, classes, spawning and waypoint upload. Native code owns perception, target memory, aiming, firing, movement, route following and basic stuck recovery. The normal `_bot_internal` and `_bot_script` thinking thread trees are bypassed. `bots_main_awareness_quality` has no effect on this controller.

Read [NATIVE-CONTROLLER.md](NATIVE-CONTROLLER.md) for the design, tradeoffs, engine bindings and measurement limits. [T5-COMPARISON.md](T5-COMPARISON.md) records the T5 engine/script research. [CAPTURE-ANALYSIS.md](CAPTURE-ANALYSIS.md) records the earlier targeting capture.

## Console test

1. Replace the plugin with this package's `_codxe/codxe.xex` and replace the entire `_codxe/games/iw3/mods/bot_warfare_native` folder. All four bot scripts changed this time. Avoid mixing in older loose/raw overrides.
2. Select `bot_warfare_native` through the existing CodXe mod workflow. For a raw-script test install, deploy the generated mod's complete `maps`, `scripts` and `scriptdata` trees through that same workflow.
3. Execute `native-bots.cfg` before loading a map. In particular, set `bots_native_controller 1`, `bots_native_drive 1` and `bots_native_profile 1`. Existing dvar values persist, so do this even though an unset dvar defaults to native mode.
4. Use Team Deathmatch, `mp_convoy`, one human and 17 bots for about three minutes again. Capture xbWatson debug output and note FPS and whether bots move, shoot and respawn.

Expected startup marker: `diagnostics=6`. The controller report now prints `capacity` (the compiled engine limit, currently 24) and `allocated` (the server's actual client allocation). During normal play, look for `controller=1`, `controller active=... paused=0 drive=1` and increasing `native_controller` / `native_sight` samples. The legacy `target`, `aim`, `movement`, script-path and target-builtin scopes should be zero in native mode because their loops are bypassed.

`sight` must be at most `frames * sight_budget`, and `paths` at most `frames * path_budget` in a reporting window. `active` counts living native bots at the last controller snapshot, so deaths and respawns change it. `graph_ready=1 nodes=238` is expected on the supplied Convoy graph. A missing graph disables roaming; it does not silently start old script AI.

If FPS remains poor, `set bots_native_drive 0` is a live diagnostic that keeps slots connected but suppresses native thinking and movement. Set it back to `1` to resume. This also changes combat, animation and FX load, so it is an isolation test rather than a pure AI-cost measurement. Use `drive 1` for the initial capture.

For the previous hybrid behaviour, set `bots_native_controller 0` **before reloading the map**. Keep the former awareness setting for that comparison. Controller ownership is selected at startup; do not toggle that setting midway through a map.

## Build and prepare

```powershell
node tools\experimental\iw3-native-bots\prepare-mod.cjs
& 'C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe' codxe.sln /m '/p:Configuration=Release' '/p:Platform=Xbox 360' /v:minimal /nologo
powershell -NoProfile -ExecutionPolicy Bypass -File tools\experimental\iw3-native-bots\package.ps1
```

The generator reads `C:\Users\Michael\code\codxenon\iw3_bot_warfare_xenon`, writes only `build/iw3-native-bots/mod`, records input hashes and checks expected source fragments. It changes four generated scripts. The source mod and game install are not modified. The packager rejects an XEX older than the native sources, then verifies every archived file against staging.

## Host checks

```powershell
clang++ -std=c++17 -O2 -Wall -Wextra -Werror tools\experimental\iw3-native-bots\test-controller.cpp -o build\iw3-native-bots\test-controller.exe
& build\iw3-native-bots\test-controller.exe
clang++ -std=c++17 -O2 -Wall -Wextra -Werror tools\experimental\iw3-native-bots\test-capacity.cpp -o build\iw3-native-bots\test-capacity.exe
& build\iw3-native-bots\test-capacity.exe
clang++ -std=c++17 -fms-extensions -DNDEBUG -O2 -Wall -Wextra -Werror -Wno-language-extension-token -I tools\experimental\iw3-native-bots\profile-test-shim tools\experimental\iw3-native-bots\test-profile.cpp -o build\iw3-native-bots\test-profile.exe
& build\iw3-native-bots\test-profile.exe
node tools\experimental\iw3-native-bots\test-integration.cjs
```

The controller test checks full-lobby budgets/fairness, filters, target retention, stale-sight firing prevention, reaction, unseen-position memory, route progression and stuck recovery. The capacity test progressively adds clients at capacities 18/24/32, reuses the highest slot and rejects invalid indices. The production-driver test uses the actual 24-slot capacity with SDK/engine stubs and checks direct trace arguments, FX gating, one lobby sweep per server time, command ownership, pause, freeze, death/respawn and reads beyond the allocated client count. Both capacity/driver tests also pass AddressSanitizer. The integration check verifies source hashes, unchanged copied assets and registered builtins.

The four generated scripts pass gsc-tool's compatible IW5 syntax parser; this is not an Xbox script runtime test. Existing `test-core.cpp` checks graph/A* and aim compatibility. `test-detour.cpp` checks the actual detour emitter against the TU4 frame prologue with a small PowerPC interpreter. The 20-byte frame-hook prologue fix remains required for valid profiling. Host timing and a successful XDK build do not establish Xbox FPS or gameplay parity.
