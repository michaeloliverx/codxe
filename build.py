import os
import subprocess
import shutil

# Configuration
# PROJECT_PATH = "."  # Current directory (adjust if needed)
SOLUTION_FILE = "iw3xe.sln"
MSBUILD_PATH = r"C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe"
# BUILD_CONFIG = "Release"
BINARY_PATH = r"build\Release\bin\iw3xe.xex"
STAGING_DIR = r"build\staging"
# RELEASE_DIR = "release"
# PROJECT_NAME = "iw3xe"
# VERSION = "1.0.0"
RESOURCES_PATH = r"resources\xenia"
GAME_TITLE_ID = "415607E6"

# Ensure MSBuild exists
if not os.path.exists(MSBUILD_PATH):
    print(f"ERROR: MSBuild not found at {MSBUILD_PATH}")
    exit(1)

VERSION_HEADER_PATH = r"src\version.h"
BUILD_NUMBER = 123

print("Generating version.h with build metadata...")
with open(VERSION_HEADER_PATH, "w") as version_file:
    version_file.write("// Auto-generated version header\n")
    version_file.write("#pragma once\n\n")
    version_file.write(f"#define BUILD_NUMBER {BUILD_NUMBER}\n")

result = subprocess.run([MSBUILD_PATH, SOLUTION_FILE], stdout=subprocess.DEVNULL)
if result.returncode != 0:
    print("ERROR: Build failed.")
    exit(result.returncode)
else:
    print("Build succeeded.")

print("Creating clean staging directory...")
if os.path.exists(STAGING_DIR):
    shutil.rmtree(STAGING_DIR)
os.makedirs(STAGING_DIR, exist_ok=True)


if os.path.exists(RESOURCES_PATH):
    # Copy the entire xenia directory and its contents to the staging directory
    shutil.copytree(RESOURCES_PATH, os.path.join(STAGING_DIR, "xenia"))
    print("Resources copied successfully")
else:
    print(f"Resources directory not found at {RESOURCES_PATH}")
    exit(1)

print("Copying binary to staging directory...")
shutil.copy2(
    BINARY_PATH,
    os.path.join(STAGING_DIR, "xenia", "plugins", GAME_TITLE_ID, "iw3xe.xex"),
)


print("Copying mods/codjumper to staging/raw directory...")
MODS_PATH = r"mods\codjumper"
RAW_DIR = os.path.join(STAGING_DIR, "raw")
if os.path.exists(MODS_PATH):
    os.makedirs(RAW_DIR, exist_ok=True)
    shutil.copytree(MODS_PATH, RAW_DIR, dirs_exist_ok=True)
    print("Mods copied successfully to staging/raw")
else:
    print(f"Mods directory not found at {MODS_PATH}")
    exit(1)
