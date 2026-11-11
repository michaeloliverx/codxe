# Bot-add crash: 0x91D5AF64

The user's xbWatson screenshot reports exception `3221225477` at address `2446700388`: access violation **0xC0000005** at **0x91D5AF64**. The user confirmed this was a rebuilt XEX, rather than the earlier versioned ZIP.

## Address trace

The local rebuilt XEX has load base `0x91D00000` while its PE/map preferred base is `0x98000000`. Therefore runtime `0x91D5AF64` maps to PE `0x9805AF64`, RVA `0x5AF64`. The preserved pre-fix map places that instruction in `Controller::Frame<ControllerWorld>`, beginning at PE `0x9805AE60`.

The faulting instruction is the native perception scheduler's indexed byte load:

```text
9805AF44: lwz    r27,0x6f0(r29)  ; sightCursor
9805AF4C: mulli  r11,r27,0x60    ; index * sizeof(Bot)
9805AF54: add    r30,r11,r29     ; &bots[index]
...
9805AF64: lbzx   r11,r11,r29     ; bots[index].enabled
```

This locates the crash in native bot-state access, before that iteration's sight query. It is not a TU4 pathfinding or collision-trace address.

The local pre-fix rebuilt XEX is timestamped 6 November 2026 12:39:37, SHA-256 `8F4CE3E1FDDC38103A57CA9FB20092B17719FF9985B2DF26B2565D8132174D68`. Its executable, map, PDB, XDB and XEX are preserved under `build/iw3-native-bots/crash-91d5af64/rebuilt-crashed-*`. No register dump was supplied; the explanation below combines the matching address, compiled layout and a reproduced overflow.

## Cause

IW3's engine extension has `MAX_CLIENTS=24`, but the new controller had its own hardcoded `Clients=18`. The script bridge accepted valid engine slots 18..23 and passed them to unchecked `Controller::Start/Stop`. Those methods wrote beyond the 18-element bot array. Controller reporting also iterated the engine limit while reading the smaller array.

The Xbox layout makes the scheduler corruption direct: `sizeof(Bot)=0x60`, so `bots[18]` begins at controller offset `0x6c0`, exactly where controller settings begin. Its `reloadAt` field at `+0x30` overlaps `sightCursor` at controller offset `0x6f0`. The compiled `Start` routine stores `now + 6000` there (`0x9805B854` / `0x9805B868` in the preserved PE). The next scheduling pass multiplies that invalid cursor by `0x60` and attempts the reported out-of-bounds byte load.

This depends on the allocated slot index, not just the number of currently connected bots. Reused/sparse slots can matter. Corrupted active/counter lines after an overflow cannot be trusted.

A host AddressSanitizer reproducer calling the old `Controller::Start(18, 20000)` reports a heap-buffer-overflow through `Controller::Bot::operator=`, `Stop` and `Start`. The report is preserved as `build/iw3-native-bots/crash-91d5af64/pre-fix-asan.txt`, alongside the pre-fix controller/core headers.

## Fix and validation

* The core now accepts an explicit capacity: production instantiates `ControllerFor<MAX_CLIENTS>`. A compile-time assertion ties the controller to the engine limit. The 18-slot alias remains only for reference host scenarios.
* `Start`, `Stop` and `Input` reject invalid indices. The native bridge/input/snapshot checks also respect `svsHeader->maxclients`, avoiding reads past the actual allocation when it is smaller than the compiled capacity.
* Actor snapshots are cleared at VM reset, preventing stale active counts before the first new scheduling pass.
* The report includes compiled `capacity` and actual `allocated` counts. The startup marker is `diagnostics=6`.
* Regression tests progressively add clients at capacities 18, 24 and 32, reuse the highest slot, reject negative/end/INT_MAX indices, and retain the global sight/path budgets. The production driver uses a 24-slot shim and verifies that slots beyond an 18-slot server allocation are rejected before dereferencing poisoned entity pointers. Both suites pass AddressSanitizer after the fix.

The gameplay/perception policy and GSC scripts are unchanged by this fix. Existing diagnostics 5 mod installs only need the new XEX and an IW3 restart. Xbox runtime confirmation of the fix remains pending.
