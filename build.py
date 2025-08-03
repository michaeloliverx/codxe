import os
import subprocess
import shutil

# Configuration
SOLUTION_FILE = "codxe.sln"
MSBUILD_PATH = r"C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe"
BINARY_PATH = r"build\Release\bin\codxe.xex"
STAGING_DIR = r"build\staging"
RESOURCES_PATH = r"resources"


def count_commits():
    try:
        result = subprocess.run(
            ["git", "rev-list", "--count", "HEAD"],
            capture_output=True,
            text=True,
            check=True,
        )

        # Get the commit count from the command output
        commit_count = result.stdout.strip()

        print(f"Total number of commits in the current branch: {commit_count}")
        return commit_count
    except Exception as e:
        print(f"An error occurred: {e}")
        exit(1)


# Ensure MSBuild exists
if not os.path.exists(MSBUILD_PATH):
    print(f"ERROR: MSBuild not found at {MSBUILD_PATH}")
    exit(1)

VERSION_HEADER_PATH = r"src\version.h"
BUILD_NUMBER = count_commits()

print("Generating version.h with build metadata...")
with open(VERSION_HEADER_PATH, "w") as version_file:
    version_file.write("// Auto-generated version header\n")
    version_file.write("#pragma once\n\n")
    version_file.write(f"#define BUILD_NUMBER {BUILD_NUMBER}\n")

result = subprocess.run([MSBUILD_PATH, SOLUTION_FILE])
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
    # Copy the contents of resources directory directly to the staging directory
    shutil.copytree(RESOURCES_PATH, STAGING_DIR, dirs_exist_ok=True)
    print("Resources directory copied successfully")
else:
    print(f"Resources directory not found at {RESOURCES_PATH}")
    exit(1)

print("Copying binary to all title ID directories...")
plugins_dir = os.path.join(STAGING_DIR, "xenia", "plugins")
if os.path.exists(plugins_dir):
    # Get all subdirectories in the plugins folder (these are the title IDs)
    title_ids = [
        d
        for d in os.listdir(plugins_dir)
        if os.path.isdir(os.path.join(plugins_dir, d))
    ]

    for title_id in title_ids:
        plugin_dir = os.path.join(plugins_dir, title_id)
        shutil.copy2(
            BINARY_PATH,
            os.path.join(plugin_dir, "codxe.xex"),
        )
        print(f"Binary copied to {title_id}")
else:
    print(f"ERROR: Plugins directory not found at {plugins_dir}")
    exit(1)


PROJECT_NAME = "codxe"
ZIP_FILE_NAME = f"{PROJECT_NAME}-r{BUILD_NUMBER}.zip"
STAGING_ZIP_PATH = os.path.join("build", ZIP_FILE_NAME)

print("Zipping the staging folder...")
shutil.make_archive(
    base_name=STAGING_ZIP_PATH.replace(".zip", ""), format="zip", root_dir=STAGING_DIR
)
print(f"Staging folder zipped successfully to {STAGING_ZIP_PATH}")
