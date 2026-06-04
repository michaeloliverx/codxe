# CoD Xe

**CoD Xe** is an open-source modding environment for the **Xbox 360** versions of older Call of Duty games.

It supports both physical Xbox 360 consoles and emulation via Xenia Canary.

WHY? This started as a fun side project to run a CodJumper mod on older Call of Duty games, then grew into a more generic modding environment for the games.

The name comes from Call of Duty and **Xe**non.

## Requirements

To run CoD Xe, you will need one of the following:

- Xbox 360 capable of running unsigned code (JTAG/RGH/DEVKIT/Bad Update)
- [Xenia Canary - Xbox 360 Emulator](https://github.com/xenia-canary/xenia-canary)

> [!IMPORTANT]
> This guide requires **Xenia Canary**. Plugin support is not available in Xenia master builds.

## Supported Games

| Game                                 | Title Update | Singleplayer | Multiplayer | Supported Region                                |
| ------------------------------------ | ------------ | ------------ | ----------- | ----------------------------------------------- |
| Call of Duty 2 (IW2)                 | TU3          | Yes          | Yes         | `Call of Duty 2 (USA, Europe)`                  |
| Call of Duty 4: Modern Warfare (IW3) | TU4          | Yes          | Yes         | `Call of Duty 4 - Modern Warfare (USA, Europe)` |
| 007: Quantum of Solace               | TU2          | Yes          | Yes         | `007 - Quantum of Solace (USA, Europe) (En,Fr)` |
| Call of Duty: World at War (T4)      | TU7          | Yes          | Yes         | `Call of Duty - World at War (USA, Europe)`     |
| Call of Duty: Modern Warfare 2 (IW4) | TU9          | Yes          | Yes         | `Call of Duty - Modern Warfare 2 (USA, Europe)` |
| Call of Duty: Modern Warfare 2 (IW4) | TU6          | No           | Yes         | `Call of Duty - Modern Warfare 2 (USA, Europe)` |
| Call of Duty: Black Ops (T5)         | TU11         | Yes          | Yes         | `Call of Duty - Black Ops (USA, Europe)`        |
| Call of Duty: Modern Warfare 3 (IW5) | TU24         | No           | Yes         | `Call of Duty - Modern Warfare 3 (USA Europe)`  |

See [Installing title updates](docs/title-updates.md) for title update installation instructions.

Feature support varies by game and mode. See the game-specific docs below for more.

## Documentation

- [Installing title updates](docs/title-updates.md)
- [Feature overview](docs/features.md)
- [Call of Duty 2 (IW2)](docs/iw2.md)
- [Call of Duty 4: Modern Warfare (IW3)](docs/iw3.md)
- [Call of Duty: World at War (T4)](docs/t4.md)
- [Call of Duty: Modern Warfare 2 (IW4)](docs/iw4.md)
- [007: Quantum of Solace](docs/qos.md)

## Included Mods

Several supported games ship with a ready-to-use `codjumper` mod in this repo:

- [`resources/iw3/_codxe/mods/codjumper`](resources/iw3/_codxe/mods/codjumper)
- [`resources/iw4/_codxe/mods/codjumper`](resources/iw4/_codxe/mods/codjumper)
- [`resources/iw5/_codxe/mods/codjumper`](resources/iw5/_codxe/mods/codjumper)
- [`resources/t4/_codxe/mods/codjumper`](resources/t4/_codxe/mods/codjumper)

These mods are practical examples of the `_codxe` mod layout and a starting point.

Several games also include an `example` mod that demonstrates the GSC loader with small, focused scripts:

- [`resources/iw2/_codxe/mods/example`](resources/iw2/_codxe/mods/example)
- [`resources/iw3/_codxe/mods/example`](resources/iw3/_codxe/mods/example)
- [`resources/iw4/_codxe/mods/example`](resources/iw4/_codxe/mods/example)
- [`resources/qos/_codxe/mods/example`](resources/qos/_codxe/mods/example)
- [`resources/t4/_codxe/mods/example`](resources/t4/_codxe/mods/example)
- [`resources/t5/_codxe/mods/example`](resources/t5/_codxe/mods/example)

## Xenia Canary Setup

Enable plugins and game-relative writes in `xenia.config.toml`:

```toml
allow_plugins = true
allow_game_relative_writes = true
```

Game-specific Xenia plugin configs and patches live under [`resources/xenia`](resources/xenia).

## Building

### Requirements

- Visual Studio 2010 Ultimate Edition
- Xbox 360 SDK

### Build

Open `codxe.sln` in Visual Studio 2010 Ultimate Edition and build the `Release|Xbox 360` configuration.

If you have `make` available, you can build the same configuration from the repository root:

```powershell
make build
```

## Credits

In some way or another, this project would not have been possible without the following people and projects:

- [callofduty4x](https://github.com/callofduty4x)
- [IW4x](https://github.com/iw4x)
- [kej](https://github.com/kejjjjj)
- [ClementDreptin](https://github.com/ClementDreptin/ModdingResources)
