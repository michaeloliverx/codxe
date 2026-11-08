# Usermap metadata

IW3 multiplayer, T4 multiplayer, and T4 single-player use the same optional
arena metadata and preview format. Each game keeps its own fastfiles under
`_codxe/games/<game>/usermaps/<map>/`.

```text
_codxe/games/iw3/usermaps/mp_example/
    mp_example.ff
    mp_example_load.ff
    mp_example_patch.ff    (optional)
    map.arena              (optional)
    preview.dds            (optional)
```

The directory and main fastfile must share the map's internal name. Both MP
browsers show only directories beginning with `mp_` that contain that main
fastfile. Companion `_load` and `_patch` directories are excluded. Adding or
removing maps updates the list the next time the custom-map menu opens.

## Arena

Save `map.arena` as plain text, preferably UTF-8 without a BOM. UTF-8 BOMs are
also accepted. `<map>.arena` is accepted as an alternative filename.

```text
{
    map "mp_example"
    longname "Example"
    description "An abandoned industrial district."
}
```

`map` must match the directory name, ignoring case. It selects metadata and
never changes the fastfile loaded. `longname` supplies the list and preview
title; `description` appears below the preview. Other keys are ignored so
standard arena files can retain fields such as `gametype`.

Quoted strings, unquoted tokens, `//` and `/* */` comments, escaped quotes,
backslashes, and `\n` line breaks are supported. A file can contain multiple
arena blocks; the first matching block is used. Metadata files are limited to
64 KiB. Missing, malformed, or nonmatching metadata does not hide a map.
MP falls back to the directory name when the arena has no longname.
T4 SP retains its existing `map.json` fallback.

## Preview

`preview.dds` must be a legacy DDS containing one **512 x 512 BC1/DXT1** image,
with no mip chain, cubemap, volume, or DX10 header. The file is 131,200 bytes:
a 128-byte header followed by 131,072 bytes of compressed pixels. Square
artwork is stretched into the menu's wide preview rectangle, as with the
stock map-selector layout. Prepare the artwork with that display in mind.

The MP browsers upload only the selected image into one resident texture.
Missing or unsupported previews are hidden, and the title and description
remain available. They never reuse the previous map's visible preview.

MP uses normal `<map>_load.ff` loading screens; no BIK playback is added.
T4 SP's existing loading-video support remains separate from this metadata.

## Shipped IW3 map metadata

`resources/iw3/_codxe/usermaps` contains metadata and 512-square previews for
the 35 maps in `codxe-iw3-fastfiles-v0.1.0.zip`. Map fastfiles remain in the
downloadable map package. Previews preserve the loading artwork embedded in
the supplied maps; Ultimate has no embedded loading artwork and currently
uses the generic blank preview. Replace its `preview.dds` when a map shot is
available.

The updated map ZIP extracts directly into `_codxe/games/iw3/usermaps` and
omits the old shared `codxe_common_mp.ff` and `codxe_ui_mp.ff` overrides.
The frontend menus now ship in the plugin's normal patch zones.
