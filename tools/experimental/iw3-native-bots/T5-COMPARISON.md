# T5 comparison and earlier IW3 console captures

This is archived research for diagnostics 3/4. The current **diagnostics 5** changes bot behaviour and requires the plugin plus four updated GSC scripts. Follow [README.md](README.md) for the current test and [NATIVE-CONTROLLER.md](NATIVE-CONTROLLER.md) for its tradeoffs. The measurement and console instructions below describe the earlier builds.

The diagnostics 3 build changed measurement coverage while retaining bot decisions, visibility tests, awareness interval and difficulty. Diagnostics 5 now replaces that active decision loop with a simpler native policy.

## Stock T5 script evidence

Inspected the user's [Plutonium T5 script dump](https://github.com/plutoniummod/t5-scripts/blob/70689b8288e60079d48926657e670c41aab4abdf/MP/Common/maps/mp/gametypes/_bot.gsc), pinned to commit `70689b8288e60079d48926657e670c41aab4abdf` (file blob `1f4987f0e261209b26a2e07b122d914faf60150c`). This is a script dump, not proof that its PC scripts and the supplied native reconstruction are the same Xbox title update.

* `bot_set_difficulty`, lines 402–527, sets native bot dvars for FOV, reaction/fire times, turning, strafing, grenades, and related behaviour. Normal uses FOV 70 and reaction range 800–1200 ms; hard uses FOV 100 and 400–700 ms. Those are difficulty choices, not a Bot Warfare-style scan-interval setting.
* `bot_is_idle`, lines 1202–1235, queries `HasScriptGoal()` and `GetThreat()` rather than implementing ordinary player perception itself.
* `bot_damage_callback`, lines 1237–1303, supplies `SetAttacker`; `bot_find_attacker`, lines 1371–1401, supplies temporary goals and special turret enemies. Other threads handle equipment, vehicles, killstreaks, and special interactions through the same native interface.

This supports keeping tactical/script control while moving low-level state and repeated work native.

## Native T5 perception evidence

Source: `C:\Users\Michael\code\KisakBlack\src\server_mp\sv_bot_mp.cpp`.

| Function | Relevant behaviour |
|---|---|
| `SV_BotThink`, line 268 | Calls native weapon, threat, sight, movement, combat, and reload updates. |
| `Bot_UpdateThreat`, line 507 | Stores per-bot threat state. Can skip the candidate rescan for a script enemy, a sufficiently close current enemy, or last stand. A damaged bot can prefer its recorded attacker. Otherwise filters active, living opposing clients before sight tests. |
| `Bot_ThreatSightCheck`, line 843 | FOV gate first, then one `SV_SightTracePoint` from the bot eye to an enemy eye/body point. Player aim points include view height and lean. This is not three skeletal tag samples with bullet/sight pairs. |
| `Bot_UpdateSight`, line 407 | Separately checks the retained threat, tracks last sight/origin state, and adjusts collision context for that target. |
| `Bot_UpdateBestThreat`, line 766 | Chooses using existing threat state, angle, distance, acquisition time, and randomness. |

The normal-player acquisition path can therefore perform less work per candidate than Bot Warfare's head/left-ankle/right-ankle checks. This is an algorithm difference as well as a native-code difference. The retained threat also has its own sight update, so this is not a claim of one trace per bot per frame. `sv_botCloseDistance` defaults to 256 in this reconstruction.

T5's eye-only acquisition could fail to acquire an enemy whose feet are exposed while the eye point is hidden. Retaining a close threat can delay selecting a different opponent. Those are inferences from the code, and are reasons to keep such policies separate from a behaviour-preserving native port. The inspected routines do not implement Bot Warfare's script `SmokeTrace` sphere test; other T5 engine functions do handle flash effects. Do not assume dropping in these two routines preserves all of Bot Warfare's smoke/flash behaviour.

## Why the existing awareness setting can affect quality

The supplied IW3 `_bot.gsc` sets awareness quality 2 to a 200 ms targeting interval. Values 0, 1, 3, and 4 select 50, 100, 400, and 800 ms. `_bot_internal::target` waits for that interval before processing candidates. Increasing it directly reduces opportunities to acquire and refresh targets and advances target memory/reaction bookkeeping in larger increments. That can reduce responsiveness. It is not equivalent to making the same calculation cheaper.

Diagnostics 3/4 did not change that setting and recorded its runtime value. Diagnostics 5 native mode does not consume it; the old hybrid still does.

## Measurement changes

* `server_frame` now wraps retail TU4 `G_RunFrame` at `0x822731F8`. The previously hooked `SV_RunFrame` wrapper at `0x82200D10` was bypassed by the server thread, as confirmed by its direct call at retail decomp line 3968588. The named debug `G_RunFrame` and KisakCOD's `void G_RunFrame(int levelTime)` establish the entry/signature. Existing first seven report slots retain their order.
* `bot_input` separately measures the valid-client portion of the existing `SV_BotUserMove` hook, including native angle update, usercmd generation, and `SV_ClientThink`. This addresses work outside `G_RunFrame`; it is not rendering or network snapshot timing.
* The script builtin resolvers wrap the original `gettagorigin`, `bullettrace`, `sighttracepassed`, and `bullettracepassed` callbacks. They pass through the same VM parameters, entity reference, return values, and original function. Timing is recorded only while the existing synchronous targeting scope is active. No extra trace is issued and no sight result is cached.
* `target_other` records each target update's elapsed time minus those four categories. It includes smoke/FOV work, script tables and selection, other unmeasured native calls, and profiling overhead. It is not a pure GSC VM cost measurement.
* `tag_hook`, `bullet_hook`, `sight_hook`, and `bulletpassed_hook` confirm whether each original builtin was observed by the resolver. A zero flag means the corresponding measurement lacks coverage; a zero row with its hook present can simply mean it was not called in that interval.
* `frame_boundary_skips` counts profiled simulation frames crossing a report/reset boundary. Those samples are omitted instead of placing their whole duration into the next window. The initial startup frame may begin before profiling is enabled and is also not sampled.

The additional categories are appended to the returned array: the original first 21 values are preserved, followed by 18 additional values. Existing experimental reporting scripts continue to work. The XEX alone can be replaced for this capture.

## Archived console test (diagnostics 3)

Use the same `mp_convoy` 9v9 setup, with the human and 17 bots. Replace the plugin, restart IW3, and capture xbWatson from startup. Confirm `diagnostics=3; frame_hook=G_RunFrame`, then set `bots_native_profile 1` before loading/restarting the map. Leave `bots_native_path`, `bots_native_aim`, `bots_native_sight_prefilter`, and `bots_main_awareness_quality` as in the previous capture. Capture roughly one to two minutes of populated gameplay and send the complete output, including startup/header lines.

There is no need for the all-zero comparison yet. This run should reveal whether the large targeting scope is chiefly skeletal lookup, collision queries, or the remaining work, and whether bot input/simulation has another large cost. Those results determine which native conversion can help without weakening awareness.
