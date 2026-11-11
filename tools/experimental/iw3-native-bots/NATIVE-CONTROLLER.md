# Native controller experiment

Current build: diagnostics 6. The client-capacity overflow found in diagnostics 5 is fixed; see [CRASH-91D5AF64.md](CRASH-91D5AF64.md). This fix changes capacity/bounds handling and resets cached actor state, without changing the perception/path budgets or gameplay policy below.

## Why change the design

The latest supplied capture (`b1b387db-da4f-4f40-9881-ee8db98ef4cc/Pasted text.txt`) reports about 10 FPS at one human and 17 bots. Across its 13 steady windows, the previous sight-first build averaged 2.800 ms per target update, versus 5.894 ms in the earlier capture. Full bullet traces fell to 4,645 of 144,713 sight queries (3.21%). These were different fights, not a controlled replay.

That build still spent about 10.23 ms in targeting, 7.77 ms in aim decisions and 3.08 ms in movement per sampled simulation frame. `G_RunFrame` averaged 40.72 ms, with a further roughly 4.80 ms of bot input/client thinking per sampled simulation frame. These are elapsed scopes, not renderer FPS or CPU utilization. Active GSC decisions remained expensive after native interpolation and A* had become cheap.

This experiment therefore removes the active script decision tree instead of translating every GSC function literally. It follows the broad T5 division of native combat and scripted setup, but is a new small IW3 controller, not a port of T5's whole bot system. It prioritizes a measurable native workload over preserving every Bot Warfare behaviour and scripting API.

## Work distribution

* `SV_BotUserMove_Stub` obtains native commands before the existing `SV_ClientThink` call. Controlled bots bypass legacy script-owned movement, buttons, mirroring and aim interpolation.
* The first native input at a new `svsHeader->time` snapshots the configured engine client slots (currently a capacity of 24) and runs one shared perception/path pass. Reads stop at the server's actual allocated client limit. Subsequent inputs at that time reuse the snapshot. Work stays on the existing server execution path; there are no new worker threads or unsynchronized engine calls.
* Fixed slot state replaces per-target GSC structs, bone lookups, trace-result arrays and per-frame script threads. Route vectors retain capacity between searches and A* reuses scratch storage. Spawns/resets and route growth can allocate/free memory; this is not a claim of zero heap use.
* Alive/team/range/FOV gates precede one eye-point query. Each bot is considered at most once per pass, with a 100 ms minimum query interval. Round robin distributes a **global six-query limit** across the lobby.
* A visible current target gets priority. A blocked current target yields to rotating candidate checks, so one enemy behind cover cannot monopolize subsequent checks. Positions update only after a successful query; pursuit uses that recorded position for at most two seconds.
* Round robin issues at most **one route search** per pass. Bots roam toward random existing waypoints or pursue recently observed positions. They do not use unseen enemy coordinates as navigation goals. C++ consumes the existing reversed A* path directly.
* Per-input work turns the view and generates local movement. Firing requires clear sight no older than 250 ms, elapsed reaction time and aim alignment. One released tick in eight permits repeated semi-auto fire. Engine empty-magazine reload remains; a six-second timer requests top-ups outside visible combat.
* Scripts reset native state at spawn/death. Native input can request respawn while dead and is suppressed during killcam archiving. One shared script gate checks prematch/end-of-match state every 250 ms. Engine frozen flags suppress active commands.

These limits bound query/search counts, not microseconds. Individual collision queries can be slow. A* retains finite node/edge/expansion limits, but is not resumable across frames in this version.

## Gameplay tradeoffs

| Area | Current behaviour / limitation |
|---|---|
| Supported test | TDM or FFA combat. Objective modes can load, but bots do not intentionally capture, plant, defuse or carry objectives. |
| Visibility | One eye point instead of three tagged bones. Cover can hide that point while leaving other body parts hittable. No bullet-result/glass exception or skeletal aim selection. |
| Smoke / flash | Uses the Xenon sight builtin's native FX visibility gate. Omits Bot Warfare's separate smoke-sphere list, proximity exceptions and script flashbang decision rules. Engine flash/weapon effects still run. |
| Awareness | Old awareness quality, revenge, footstep, UAV and teammate-alert logic are unused. Acquisition is visual and scheduled; nearby enemies can qualify outside the cone. Occluded candidates can take several passes to find. |
| Targeting | One global reaction/turn/range configuration. Old skill presets, aim noise, dynamic bone choice and script-forced enemies are not consumed. A visible current target is retained until blocked or invalid. |
| Navigation | Basic waypoint roaming, pursuit and stuck recovery. Uploaded jump/crouch/prone/climb annotations receive approximate handling. No original mantle probes, detailed ladder logic, grenade avoidance, congestion accounting, nearest-visible endpoint selection or local crowd avoidance. Bots may bunch up or get stuck. |
| Combat | Class-selected weapon, ADS/fire/reload. No tactical weapon selection, ammo scavenging, melee policy, grenade/equipment use, vehicle targeting or killstreak activation. Semi-auto cadence and sniper behaviour differ. |
| Scripting | Team/class/spawn/menu management remains. Old goal/aim/enemy/freeze utility APIs do not control native behaviour. Engine freeze flags and shared pause still apply. Mode is latched at map load. |
| Invalid graph | No script-AI fallback in native mode. Combat remains available, but roaming requires an accepted graph; check report status. |

These omissions define this experiment, not an inherent limit of native bots. Behaviour can be restored selectively after measuring its cost.

## Settings

Execute generated `native-bots.cfg` before the test. Unset dvars receive these defaults; existing values persist.

| Dvar | Default | Meaning |
|---|---:|---|
| `bots_native_controller` | 1 | Select at map load; `0` restores the previous hybrid on reload. Editor mode uses the hybrid. |
| `bots_native_profile` | 1 | Periodic xbWatson/DbgPrint reports, enabled during script initialization. |
| `bots_native_drive` | 1 | Live diagnostic: `0` suppresses native thinking/input and retains connected slots. |
| `bots_native_sight_budget` | 6 | Global sight/FX queries per pass; clamped 1..18. |
| `bots_native_path_budget` | 1 | Global A* searches per pass; clamped 0..2. Zero prevents new routes; existing routes can still be followed. |
| `bots_native_reaction_ms` | 250 | New-target reaction delay; 0..2000. |
| `bots_native_range` | 4000 | Acquisition range in world units; 256..12000. |
| `bots_native_fov` | 180 | Horizontal acquisition cone in degrees; 30..360. Current-target rechecks can retain an enemy outside it. |
| `bots_native_turn_rate` | 300 | Pitch/yaw turn speed in degrees per second; 30..1080. |

Live tuning settings refresh together every 500 ms. `bots_native_path`, `bots_native_aim`, the sight-prefilter/order switches and awareness quality still affect the **legacy hybrid**. Native controller mode always uploads/uses its graph and its own aim/perception code.

## Retail TU4 evidence

Retail `sub_8225C318` matches named debug `Scr_SightTracePassed`. It calls `sub_82355488` with an input/output hit integer, start/zero mins/zero maxs/end, two ignored entity IDs and a mask. This retail point-only implementation calls world sight collision and entity sight collision. The native wrapper initializes hit to zero, ignores observer/target and uses `0x801803`, the stock sight builtin's mask with player collision disabled. It retains server/world/static-model collision through the server query rather than using a world-only box trace.

When clear, retail calls `sub_8228F868` with the integer at `0x82725F64` (`fx_serverVisClient`) and the same endpoints, accepting visibility >= 0.2. The native wrapper uses the same `double(int, const float*, const float*)` interface, confirmed by KisakCOD `EffectsCore/fx_system.cpp` and the retail decomp. The host driver test checks arguments, initialized hit state, mask and FX short-circuit/threshold; it does not execute Xbox engine code.

`SetClientViewAngle` remains the verified TU4 binding at `0x82284C60`. Generated command angles copy `sess.cmd.angles`, the baseline that function uses to derive `ps.delta_angles`. `G_RunFrame` profiling retains the verified 20-byte detour prologue. Native control starts only through the generated bot spawn calls; ordinary IW3 bots and other games do not automatically start it.

## Reading the next capture

`native_controller` includes shared snapshot/perception/planning plus per-bot commands inside `bot_input`. `native_sight` is a subset of `native_controller`. Do not add nested scopes to their parents.

Compare **both `server_frame` and `bot_input`** against the previous capture: active AI moved from GSC during `G_RunFrame` into the input path. A lower server-frame number alone would be misleading. Normalize using window duration, frame counts and bot-input call counts.

Counters report passes, queries, paths, acquisitions, stuck detections and failed routes. Zero legacy AI scopes confirm bypassed loops; zero legacy trace-hook flags can also be normal when scripts no longer reference those builtins. Active counts vary with deaths/respawns. These scopes do not measure client rendering/GPU time. Player simulation, animation, snapshots, combat FX and rendering remain engine costs at 18 players.
