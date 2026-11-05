# IW3 MP experimental demos

This prototype targets Xbox 360 IW3 MP TU4. It records game state for later rendering, including the local host's view. It does not record video or replay movement inputs through the server.

## Record a run

Load a map with one local player, then enter:

```text
demorecord jump01
```

Play normally, then save:

```text
demostoprecord
```

`demorecord` without a name selects the first unused `demo0000` through `demo9999`. `record` is an alias. Names accept letters, digits, underscores and hyphens. Supplying an existing name overwrites that recording on disk. Leaving the level closes the recording automatically. `stopdemo` also saves an active recording.

The existing TAS movement recorder owns `startrecord`, `stoprecord` and `togglerecord`. Those commands remain unchanged; use `demostoprecord` or `stopdemo` to save a demo.

Files live at `_codxe/games/iw3/demos/<name>.xedemo`, relative to the loaded CoD Xe installation. Recording is available while hosting or joining a game. Split-screen recording is not supported.

## Play and edit the shot

Load the **same map and mod** in a local host session, with one local player and no remote players connected. Bots are allowed. Then:

```text
demo jump01
demo_timescale 0.05
cg_fov 80
r_filmUseTweaks 1
r_filmTweakEnable 1
r_filmTweakContrast 1.2
r_filmTweakDesaturation 0.2
```

The demo is rendered again with current settings. `demo_timescale` controls playback independently of the server and the engine's `timescale`; its range is `0.001` to `16`. FOV, film tweaks, `cg_draw2D` and `cg_drawGun` remain editable during playback, including when paused. Their original protection flags return when playback ends; chosen visual values remain applied.

| Command / dvar | Effect |
| --- | --- |
| `demopause` | Toggle pause. |
| `demoseek 12.5` | Seek to 12.5 seconds from the beginning. Rebuilds the cgame and processes snapshots/commands from the start. |
| `demorestart` | Rebuild and replay from the beginning, preserving the pause state. |
| `demostatus` | Show position, duration, speed and snapshot count. |
| `stopdemo` | Save an active recording, or end playback and rebuild the live cgame from the saved client state. |
| `demo_timescale 0.01` | Render at one hundredth speed. |
| `demo_smoothCamera 1` | Use recorded render-frame player states, interpolating the camera between them. Default on. |
| `demo_smoothCamera 0` | Use the engine's snapshot interpolation instead; useful for comparison. |
| `demo_captureFps 60` | Advance by exactly `1000 / 60 * demo_timescale` milliseconds per rendered frame. |
| `demo_captureFps 0` | Return to real-time playback timing. Default. |
| `demo_holdAtEnd 1` | Hold the final frame for capture instead of returning to the live match. Default off. |

Playback returns to the live match when it reaches the end. With `demo_holdAtEnd 1`, it pauses on the final frame without the live connection-interrupted warning. `demorestart` preserves that paused state; use `demopause` to resume. For a clean shot, `cg_draw2D 0` hides the native HUD, and `cg_drawGun 0` hides the weapon if desired.

`demo_captureFps` only controls the demo clock. Capture frames/video separately at the selected output rate. It does not write screenshots, synchronize an external recorder, increase Xenia resolution or add motion blur. Start the capture, seek/restart while paused, then unpause. Disk or render stalls do not advance a fixed-step demo by extra time.

## Current boundaries

- Playback requires an already loaded matching map/mod. Automatic offline map loading is future work. The local listen server is suspended during playback, and client packet writes/received server-message parsing are suppressed. Remote peers are rejected before playback starts.
- These are TU4 Xbox `.xedemo` files, incompatible with PC `.dm_1` files and IW3SR server demos. Use the same game assets and mod version as the recording. The header checks the file version and engine structure sizes, not asset contents.
- The recording includes the original client's visible snapshot data and view. It does not provide a freely selectable viewpoint or recover entities outside that client's snapshots.
- Camera samples are captured at the original render rate; world entities retain the original snapshot rate. Slower playback interpolates those samples. It cannot recover detail that was never sampled. The native world/FX clock still uses integer milliseconds; the recorded player camera interpolates using fractional demo time.
- Seeking rebuilds client state and replays commands. FX, audio, animation random seeds, dynamic physics and viewmodel transients are not a saved engine checkpoint. Expect differences, especially after a seek. For a final take, replay from the start with some lead-in.
- Payloads are uncompressed. A camera sample alone is 12,136 bytes; at 60 samples/sec that is about 0.69 MiB/sec, plus snapshots and commands. Files stop at the CRT's 2 GB addressing limit. The reader can recover complete records if the last write was interrupted.
- Resume-after-playback, host/client capture parity, controller/console behavior during playback and long recordings need runtime testing. Build and desktop stream tests do not establish those behaviors.

## Implementation and evidence

The experiment lives in `src/game/iw3/mp/components/demo.cpp`, with a portable envelope/timeline implementation in `demo_stream.h`. It streams variable-length snapshots, reliable server commands and per-render predicted player states to disk. Playback indexes record offsets, keeps a pair of camera samples and one snapshot cached, and uses the original cgame's snapshot transitions/rendering. Client state is saved before playback and restored afterward.

The supplied symbol-rich `iw3_472_debug_default_demo_mp.xex.c/.h` identifies the functions and layouts. The retail `iw3_default_mp_tu4.xex.c` supplies the addresses and TU4 sizes. The debug build's client structures are not used as a substitute for retail addresses/layouts.

| Retail TU4 address | Boundary |
| --- | --- |
| `0x822D20A8` | `CL_Frame`: bypass live connection/time handling during playback. The retail main loop calls this function; `0x822D1870` is the reconnect helper it calls. |
| `0x822CF200` | `CL_ClearState`: close a recording/player before the client is cleared. |
| `0x822E6110` | `CL_GetSnapshot`: capture decoded snapshots; supply recorded snapshots on replay. |
| `0x822E6578` | `CL_CGameNeedsServerCommand`: supply archived commands to native command processing. |
| `0x822CBF18` | `CL_ParseServerMessage`: exclude live messages during replay. |
| `0x822DA6C8` | `CL_WritePacket`: suppress replay input reaching the local server. |
| `0x822F4DA0` | `CG_DrawActiveFrame`: feed the demo clock and `DEMO_TYPE_CLIENT`. |
| `0x82301A28` | `CG_PredictPlayerState`: capture/restore the rendered player's state. |
| `0x8231AC70` | `CG_DrawDisconnect`: suppress the live command-acknowledgment warning during demos. |
| `0x82309A90` | `CG_Shutdown`: close demos on cgame teardown; rebuild on seeks. |
| `0x82201A70` | `SV_Frame`: suspend the local listen server during replay. |

The important surviving engine behavior is `CG_PredictPlayerState_Internal`: a nonzero `cg.demoType` selects `CG_InterpolatePlayerState` instead of live input prediction. `CG_ProcessSnapshots` reads `clientActive_t::snap` directly in retail, so hooking `CL_GetCurrentSnapshotNumber` alone would miss it. The player updates that metadata as well as supplying `CL_GetSnapshot`.

KisakCOD's PC client demo path supplies the architectural reference: startup configstrings, reliable commands, snapshots and local predicted-data history. IW3SR's supplied `ServerDemo.cpp` is a client-side reader for extra server-demo telemetry; the recording server itself is not implemented there. This experiment uses the Xbox client's decoded snapshot boundary so recording also works for its local listen server, without transplanting a PC dedicated-server system.

Run the portable format/clock tests with:

```powershell
tools\test-iw3-demo.cmd
```

They cover file/version/layout rejection, interrupted final records, command continuity beyond 128 entries, timeline lookup, angle wrapping, pause, millisecond counter wrap, fractional slow motion and fixed capture timing. Xbox Release/Debug builds validate compilation, linking and layout assertions. Live smoke test: record a short jump, stop, replay at normal speed, try `0.05` and `0.001`, pause/change FOV and film tweaks, seek in both directions, restart, stop/resume, then repeat a capture as a remote client.
