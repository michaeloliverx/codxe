"""Build the monitor and title modules, then create Xbox 360 and Xenia ZIPs."""
import argparse
import re
import shutil
import struct
import subprocess
import tomllib
from pathlib import Path

ROOT = Path(__file__).resolve().parent
BUILD = ROOT / "build"
MSBUILD = Path(r"C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe")
RESOURCES = ROOT / "resources"
IGNORE_CONFIGS = shutil.ignore_patterns("codxe.json", "codxe.example.json")
# Title IDs, resource directories and XEX variants to include in the packages.
# Keep this list in sync with the supported executables in src/module/games.h.
PACKAGED_GAMES = {
    0x415607D1: ("iw2", ("sp", "mp")),
    0x415607E1: ("ngl", ("mp",)),
    0x415607E6: ("iw3", ("sp", "mp")),
    0x415607FF: ("qos", ("sp", "mp")),
    0x4156081C: ("t4", ("sp", "mp")),
    0x41560817: ("iw4", ("sp", "mp", "mp.tu6")),
    0x41560855: ("t5", ("sp", "mp")),
    0x415608CB: ("iw5", ("mp",)),
}
REQUIRED_ZONES = {
    "iw4": ("patch_mp",),
    "iw3": ("patch_mp",),
    "t4": ("patch", "patch_ui", "patch_mp", "patch_ui_mp"),
}


def game_modules():
    entries = []
    for title, (game, variants) in PACKAGED_GAMES.items():
        for variant in variants:
            entries.append(dict(title=title, game=game, filename=f"codxe.{game}.{variant}.xex",
                                name=f"{game}_{variant.replace('.', '_')}"))
    return entries


def read_git_version(configuration):
    header = BUILD / configuration / "obj/codxe/git_version.h"
    match = re.search(r'^#define GIT_VERSION "([^"]+)"', header.read_text(), re.M)
    if not match:
        raise RuntimeError(f"Generated version missing in {header}")
    return match.group(1)


def inspect_xex(path, flags, revision):
    data = path.read_bytes()
    if len(data) < 24 or data[:4] != b"XEX2" or struct.unpack_from(">I", data, 4)[0] & 0xB != flags:
        raise RuntimeError(f"Incorrect XEX process flags in {path}")
    count = struct.unpack_from(">I", data, 20)[0]
    if 24 + count * 8 > len(data):
        raise RuntimeError(f"Truncated XEX header in {path}")
    for index in range(count):
        key, offset = struct.unpack_from(">II", data, 24 + index * 8)
        if key == 0x40006 and offset + 24 <= len(data):
            if struct.unpack_from(">I", data, offset + 4)[0] != revision << 8:
                raise RuntimeError(f"Incorrect native execution-info version in {path}")
            return
    raise RuntimeError(f"Execution-info header missing in {path}")


def audit(configuration, entries):
    release = read_git_version(configuration)
    revision = int(re.match(r"r(\d+)", release).group(1))
    output = BUILD / configuration / "bin"
    inspect_xex(output / "codxe.xex", 0xA, revision)
    symbols = (output / "codxe.map").read_text(errors="replace")
    if re.search(r"(?i)libcmt|libcpmt|\bmalloc\b|\bfree\b|HeapAlloc|GetProcessHeap|"
                 r"DllMainCRTStartup|CxxFrameHandler|\bxapilib(?:d)?\b|__IMPORT_DESCRIPTOR_xam|\.CRT\$", symbols):
        raise RuntimeError("Resident monitor acquired a CRT, heap or static XAM dependency")
    for entry in entries:
        inspect_xex(output / entry["filename"], 0x9, revision)
        header = BUILD / configuration / "obj" / entry["name"] / "git_version.h"
        if f'#define GIT_VERSION "{release}"' not in header.read_text():
            raise RuntimeError(f"Module version differs from monitor: {entry['name']}")
    print(f"Verified heap-free monitor and {len(entries)} title modules, native version 0.0.{revision}.0")
    return release


def copy_game_data(game, destination):
    source = RESOURCES / game / "_codxe"
    if source.is_dir():
        shutil.copytree(source, destination, ignore=IGNORE_CONFIGS)
    (destination / "mods").mkdir(parents=True, exist_ok=True)
    (destination / "usermaps").mkdir(exist_ok=True)


def verify_xenia(stage, entries):
    expected = {(entry["title"], "_codxe/modules/" + entry["filename"]) for entry in entries}
    actual = set()
    for config in (stage / "plugins").glob("*/plugins.toml"):
        title = int(config.parent.name, 16)
        document = tomllib.loads(config.read_text())
        if int(document["title_id"], 16) != title:
            raise RuntimeError(f"Title ID differs from plugin directory: {config}")
        for plugin in document.get("plugin", []):
            if plugin.get("is_enabled", True):
                route = (title, plugin["file"])
                if route not in expected or not (config.parent / plugin["file"]).is_file():
                    raise RuntimeError(f"Invalid or missing module in {config}: {plugin['file']}")
                if route in actual:
                    raise RuntimeError(f"Duplicate enabled module in {config}: {plugin['file']}")
                actual.add(route)
    if actual != expected:
        raise RuntimeError("Xenia configs do not cover every supported executable")


def package(configuration, entries, release):
    # Shared zones remain versioned release resources, rebuilt separately.
    for game, names in REQUIRED_ZONES.items():
        for name in names:
            zone = RESOURCES / game / "_codxe/zone" / (name + ".ff")
            if not zone.is_file() or not zone.stat().st_size:
                raise FileNotFoundError(f"Required {game.upper()} zone missing or empty: {zone}")

    output = BUILD / configuration / "bin"
    for platform in ("xbox360", "xenia"):
        stage = BUILD / "staging" / platform
        if stage.exists():
            target = stage.resolve()
            if not target.is_relative_to(ROOT / "build") or target.parent != (BUILD / "staging").resolve():
                raise RuntimeError(f"Refusing to clear staging directory outside build/staging: {target}")
            shutil.rmtree(stage)
        stage.mkdir(parents=True)
        if platform == "xenia":
            shutil.copytree(RESOURCES / "xenia", stage, ignore=IGNORE_CONFIGS, dirs_exist_ok=True)
        else:
            shared = stage / "_codxe"
            (shared / "modules").mkdir(parents=True)
            shutil.copy2(output / "codxe.xex", shared / "codxe.xex")

        data_directories = set()
        for entry in entries:
            game = entry["game"]
            if platform == "xenia":
                shared = stage / "plugins" / f"{entry['title']:08X}" / "_codxe"
            data = shared if platform == "xenia" else shared / "games" / game
            if data not in data_directories:
                copy_game_data(game, data)
                data_directories.add(data)
            (shared / "modules").mkdir(parents=True, exist_ok=True)
            shutil.copy2(output / entry["filename"], shared / "modules" / entry["filename"])

        for game in sorted({entry["game"] for entry in entries}):
            updates = RESOURCES / game / "Title Updates"
            if updates.is_dir():
                shutil.copytree(updates, stage / "title-updates" / game, ignore=IGNORE_CONFIGS)

        if platform == "xenia":
            verify_xenia(stage, entries)
        (stage / "LICENSES").mkdir()
        shutil.copy2(ROOT / "LICENSE", stage / "LICENSES/codxe.txt")
        shutil.copy2(ROOT / "third_party/xkelib/LICENSE", stage / "LICENSES/xkelib.txt")
        suffix = "-debug" if configuration == "Debug" else ""
        archive = BUILD / f"codxe-{release}{suffix}-{platform}"
        shutil.make_archive(str(archive), "zip", root_dir=stage)
        print(f"{platform} folder: {stage}")
        print(f"{platform} ZIP: {archive}.zip")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--configuration", choices=("Debug", "Release"), default="Release")
    action = parser.add_mutually_exclusive_group()
    action.add_argument("--build-only", action="store_true")
    action.add_argument("--package-only", action="store_true")
    args = parser.parse_args()
    entries = game_modules()
    if not args.package_only:
        subprocess.run([
            str(MSBUILD), str(ROOT / "codxe.sln"), "/m:4", "/nologo", "/v:minimal",
            "/p:Configuration=" + args.configuration, "/p:Platform=Xbox 360",
        ], cwd=ROOT, check=True)
    release = audit(args.configuration, entries)
    if not args.build_only:
        package(args.configuration, entries, release)


if __name__ == "__main__":
    main()
