# Installing Title Updates

CoD Xe expects the title update listed in the README's supported games table to be active. The matching title update files are included under `resources/<game>/Title Updates`.

## Bundled Title Updates

| Game                                        | Title ID   | Required TU | File                                                                                                                                                  |
| ------------------------------------------- | ---------- | ----------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- |
| Call of Duty 2 (IW2)                        | `415607D1` | TU3         | [`resources/iw2/Title Updates/TU3/TU_10LC1UH_0000000000000.0000000000181`](/resources/iw2/Title%20Updates/TU3/TU_10LC1UH_0000000000000.0000000000181) |
| Call of Duty 4: Modern Warfare (IW3)        | `415607E6` | TU4         | [`resources/iw3/Title Updates/TU4/TU_10LC1V6_0000004000000.0000000000101`](/resources/iw3/Title%20Updates/TU4/TU_10LC1V6_0000004000000.0000000000101) |
| 007: Quantum of Solace                      | `415607FF` | TU2         | [`resources/qos/Title Updates/TU2/TU_10LC1VV_000000S000000.00000000000G7`](/resources/qos/Title%20Updates/TU2/TU_10LC1VV_000000S000000.00000000000G7) |
| Call of Duty: World at War (T4)             | `4156081C` | TU7         | [`resources/t4/Title Updates/TU7/TU_10LC20S_0000010000000.00000000001O8`](/resources/t4/Title%20Updates/TU7/TU_10LC20S_0000010000000.00000000001O8)   |
| Call of Duty: Modern Warfare 2 (IW4 TU9)    | `41560817` | TU9         | [`resources/iw4/Title Updates/TU9/TU_10LC20N_0000018000000.000000000028A`](/resources/iw4/Title%20Updates/TU9/TU_10LC20N_0000018000000.000000000028A) |
| Call of Duty: Modern Warfare 2 (IW4 MP TU6) | `41560817` | TU6         | [`resources/iw4/Title Updates/TU6/TU_10LC20N_0000018000000.00000000001GA`](/resources/iw4/Title%20Updates/TU6/TU_10LC20N_0000018000000.00000000001GA) |
| Call of Duty: Black Ops (T5)                | `41560855` | TU11        | [`resources/t5/Title Updates/TU11/tu00000003_00000000`](/resources/t5/Title%20Updates/TU11/tu00000003_00000000)                                       |
| Call of Duty: Modern Warfare 3 (IW5)        | `415608CB` | TU24        | [`resources/iw5/Title Updates/TU24/tu00000002_00000000`](/resources/iw5/Title%20Updates/TU24/tu00000002_00000000)                                     |

## Xbox 360

If you have a stealth server setup then you can simply update the game via Xbox Live.

Title update install location depends on the filename format:

- Uppercase `TU_...` files go in `Hdd1:\Cache`.
- Lowercase `tu...` files go in `Hdd1:\Content\0000000000000000\<TitleID>\000B0000`.

Examples:

```text
Hdd1:\Cache\TU_10LC1V6_0000004000000.0000000000101
Hdd1:\Content\0000000000000000\41560855\000B0000\tu00000003_00000000
```

## Xenia Canary

The easiest method is Xenia Canary's content installer:

1. Open Xenia Canary.
2. Run the game once if you need to confirm the title ID.
3. Select `File` > `Install Content`.
4. Select the title update file from `resources/<game>/Title Updates/<TU>/`.
5. Restart the game.

## References

- [ConsoleMods: Xbox 360 Title Updates](https://consolemods.org/wiki/Xbox_360:Title_Updates)
- [Xenia Canary Quickstart](https://github.com/xenia-canary/xenia-canary/wiki/Quickstart)
- [Xenia Canary FAQ](https://github.com/xenia-canary/xenia-canary/wiki/FAQ)
