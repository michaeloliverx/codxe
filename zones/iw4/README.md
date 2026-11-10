IW4 TU6 menu sources extend the stock `patch_mp` zone. The asset list records the
original patch contents; stock startup dependencies remain references. Runtime
selectors use the small shared header in `raw/ui_mp/menudef.h`.

The TU6 input is from `TU_10LC20N_0000018000000.00000000001GA`.
The similarly named package ending in `028A` is TU9.

Run `build.ps1` with `-Linker`, `-GameDirectory`, and `-StockPatch` pointing to the
Xenon-capable OAT linker and original game/TU6 inputs. Built fastfiles are copied
to `resources/iw4/_codxe`. Packaging ships these versioned outputs directly.

The build extracts stock menu dependencies under `build/iw4-zones` using the OAT
unlinker beside the linker (override with `-Unlinker`). It appends the two browsers
to the generated stock patch menu list. Only menus we edit live in `zone_raw`;
unchanged stock menus and their shared function definitions remain generated inputs.

User maps live in `games/iw4/usermaps/<mp_name>/` with `<mp_name>.ff`, an optional
`<mp_name>_load.ff`, `map.arena` (or `<mp_name>.arena`), and optional `preview.dds`.
Previews are 512x512 DXT1 DDS files with one mip. Arena keys are preserved, including
IW4 faction, environment and map-image fields. Map download data stays outside Git.
