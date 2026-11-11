# Console capture: perception breakdown and diagnostics 4

This records the earlier targeting investigation. The current diagnostics 5 native controller and latest-capture rationale are documented in [NATIVE-CONTROLLER.md](NATIVE-CONTROLLER.md); follow [README.md](README.md) for installation/testing.

Input: the user's 6 November 2026 `mp_convoy` capture, with one human and 17 bots. The runtime header confirms awareness quality 2, `sv_fps=20`, all three original optimization switches enabled, and a 238-node native graph. All four target-builtin hooks were observed.

The following totals use the twelve populated reporting windows after the initial spawn ramp, covering 166.675 seconds of wall time and 8,897 target updates. They are elapsed scope times, not CPU utilization or renderer FPS. Different fights and profiling overhead prevent treating the previous capture as a controlled speed comparison.

| Targeting component | Total | Share of targeting | Mean per callback |
|---|---:|---:|---:|
| Full `bullettrace` | 26.683 s | 50.9% | 192.5 us |
| Remaining target work | 18.817 s | 35.9% | 2,115 us per target update |
| `sighttracepassed` | 3.644 s | 6.9% | 26.3 us |
| `gettagorigin` | 3.256 s | 6.2% | 23.5 us |
| Special-target `bullettracepassed` | 0.042 s | 0.08% | 221.4 us |

Total targeting averaged 5.894 ms/update and reached 18.031 ms in these windows. There were 138,635 calls each to tag lookup, full bullet trace, and sight trace: about 15.6 bone samples per target update. The original helper always performs a full bullet trace before discovering that sight failed. Each full trace averaged over seven times the elapsed time of a sight check in this capture. These callback timings include argument handling and construction of the script trace-result object as well as collision work; they do not isolate the native collision algorithm alone.

Bot input averaged 284.8 us across 40,800 calls, with a worst sample of 17.769 ms. Native A* averaged 30.3 us across 710 searches. Routing and native aim setup are not the leading measured costs. The unclassified targeting remainder includes smoke/FOV, script arrays and selection, other native calls, and measurement overhead; it is not all VM dispatch.

## Query-order change

Before: fetch the same bone, issue a full bullet trace, then require both sight success and a clear/glass bullet result.

After: fetch the same bone, test sight, and return false immediately when blocked. If sight passes, issue the same bullet trace and apply the same clear/glass rule. This preserves `sight && (clear || glass)`, all three bone samples and their short-circuit order, and the awareness interval. It changes the order of pure collision queries and skips a redundant query after failed sight. `bots_native_sight_order=0` restores the old order for comparison.

No performance factor is claimed yet: the benefit depends on how often those sight checks fail. The new log should show fewer full bullet traces than sight tests when blocked candidates are common. A native-only conversion could reduce much of the remaining VM/bookkeeping overhead, but would still pay for collision queries, so this independent optimization is worth testing first.

## Why frame samples were still zero

`frame_boundary_skips=200` in every report proves that the new frame hook was executing, but its saved generation value did not survive the call to the original function. This was not 200 actual report resets.

The matching local TU4 image was extracted read-only and disassembled. Its `G_RunFrame` prologue at `0x822731F8` is:

```text
mflr r12
bl __savegprlr_19
stfd fr30,-0x80(r1)
stfd fr31,-0x78(r1)
stwu r1,-0x930(r1)
```

`__savegprlr_19` saves r27 at `-0x30(r1)`. The existing detour copied only the first 16 bytes, then used that same stack slot to preserve its scratch register while jumping back. Because `stwu` had not executed, that write replaced the saved r27. The Release frame hook held its generation snapshot in r27, as confirmed by its disassembly, so every return appeared to cross a report boundary.

The detour API now accepts an optional minimum copied-prologue length. This hook requests 20 bytes so the copied `stwu` allocates the callee frame before the preserving jump back. Other callers retain the existing 16-byte default. The actual-emitter regression test reproduces the old corruption and verifies r27 and the other nonvolatile registers with the extended copy, including unhook restoration. This is a prologue-length fix, rather than disabling the reporting guard or making its local variable volatile.

## Next test

Install the diagnostics 4 XEX **and** matching generated scripts. Keep the same map, 17 bots, and awareness quality; enable profiling and leave `bots_native_sight_order=1`. Capture startup plus one to two minutes. Expected evidence is a lower full-bullet call count than sight-call count, populated simulation samples with roughly one report-boundary skip per interval, and the user's assessment of responsiveness and bot behaviour. Retest on hardware remains necessary.
