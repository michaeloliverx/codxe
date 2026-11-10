"""Build the plugin and package its shared storage tree for Xbox 360 and Canary."""
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MSBUILD_PATH = Path(r"C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe")
BINARY_PATH = ROOT / "build/Release/bin/codxe.xex"
STAGING_DIR = ROOT / "build/staging"
RESOURCES_PATH = ROOT / "resources"
VERSION_HEADER_PATH = ROOT / "build/Release/obj/git_version.h"
GAME_DIRECTORIES = {
    "415607D1": "iw2",
    "415607E1": "ngl",
    "415607E6": "iw3",
    "415607FF": "qos",
    "4156081C": "t4",
    "41560817": "iw4",
    "41560855": "t5",
    "415608CB": "iw5",
}
REQUIRED_ZONES = {
    "iw4": ("patch_mp",),
    "iw3": ("patch_mp",),
    "t4": ("code_post_gfx", "common", "patch", "patch_ui", "patch_mp", "patch_ui_mp"),
}


def read_git_version():
    for line in VERSION_HEADER_PATH.read_text().splitlines():
        if line.startswith("#define GIT_VERSION "):
            return line.split('"')[1]
    raise RuntimeError(f"GIT_VERSION was not found in {VERSION_HEADER_PATH}")


def copy_game_data(game, destination):
    shutil.copytree(RESOURCES_PATH / game / "_codxe", destination,
                    ignore=shutil.ignore_patterns("codxe.json"))
    (destination / "mods").mkdir(exist_ok=True)
    (destination / "usermaps").mkdir(exist_ok=True)


def package():
    # Shared zones are versioned release resources; rebuilding remains manual.
    for game, names in REQUIRED_ZONES.items():
        for name in names:
            zone = RESOURCES_PATH / game / "_codxe/zone" / (name + ".ff")
            if not zone.is_file() or not zone.stat().st_size:
                raise FileNotFoundError(f"Required {game.upper()} zone missing or empty: {zone}")

    if STAGING_DIR.exists():
        if STAGING_DIR.resolve().parent != (ROOT / "build").resolve():
            raise RuntimeError("Refusing to clear a staging directory outside build/staging")
        shutil.rmtree(STAGING_DIR)
    shared = STAGING_DIR / "_codxe"
    shared.mkdir(parents=True)
    shutil.copy2(BINARY_PATH, shared / "codxe.xex")
    shutil.copytree(RESOURCES_PATH / "xenia", STAGING_DIR / "xenia")

    for title, game in GAME_DIRECTORIES.items():
        copy_game_data(game, shared / "games" / game)
        plugin = STAGING_DIR / "xenia/plugins" / title / "_codxe"
        plugin.mkdir()
        shutil.copy2(BINARY_PATH, plugin / "codxe.xex")
        copy_game_data(game, plugin / "games" / game)
        updates = RESOURCES_PATH / game / "Title Updates"
        if updates.is_dir():
            shutil.copytree(updates, STAGING_DIR / "title-updates" / game)

    shutil.copy2(ROOT / "LICENSE", STAGING_DIR / "LICENSE")
    licenses = STAGING_DIR / "LICENSES"
    licenses.mkdir()
    shutil.copy2(ROOT / "third_party/xkelib/LICENSE", licenses / "xkelib.txt")
    archive = ROOT / "build" / ("codxe-" + read_git_version())
    shutil.make_archive(str(archive), "zip", root_dir=STAGING_DIR)
    print(f"Test folder: {STAGING_DIR}")
    print(f"ZIP: {archive}.zip")


def main():
    subprocess.run([
        str(MSBUILD_PATH), str(ROOT / "codxe.sln"), "/m", "/p:BuildInParallel=true",
        "/p:Configuration=Release", "/p:Platform=Xbox 360",
    ], cwd=ROOT, check=True)
    package()


if __name__ == "__main__":
    main()
