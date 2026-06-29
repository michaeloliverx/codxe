# CoD Xe v2 spike

This tree is the clean architecture experiment. It intentionally does not include v1 headers or modules.

Current scope:

- plugin context
- platform detection
- target detection
- fixed lifecycle callback bus
- script loading feature entry point through `Scr_AddSourceBuffer`
- asset override feature entry point through `DB_LinkXAsset`
- logging through `DbgPrint`
- bounded path/file helpers
- IW3 MP target metadata and lifecycle address table

Current non-goals:

- no feature parity with v1
- no image loading
- no MPSP
- no script loading yet
- no lifecycle detour installation yet
