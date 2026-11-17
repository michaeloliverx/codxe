# T4 SP shared zones

CodXe-owned replacement zones for World at War TU7 (USA/Europe).

| Zone | Purpose |
| --- | --- |
| `patch.ff` | Extra Settings with FOV controls, a split-screen layout toggle and sidebar materials using the stock `$black` image. |
| `patch_ui.ff` | Custom-map browser entry points and its reusable preview material. |

Built fastfiles live in `resources/t4/_codxe/zone/`; release packaging copies
them to `t4/_codxe/zone/`. Install the plugin and zones together. Map downloads
contain only `usermaps/<map>/` and must not overwrite shared zones.

`zone_source/` contains the zone definitions and stock asset lists;
`zone_raw/` contains the custom source assets. Rebuilding is manual with a
T4 Xenon-capable OpenAssetTools Linker/Unlinker and original retail/TU7 inputs.
Update sources and built fastfiles together.

Install the release's `t4/_codxe` folder in the game directory as `_codxe/`.
The client reads configuration, mods, zones and maps from `game:/_codxe/`.

See the [usermap format](../../docs/t4-usermaps.md) for map-owned files.
