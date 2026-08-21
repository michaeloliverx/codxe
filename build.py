import os
import subprocess
import shutil

# Configuration
SOLUTION_FILE = "codxe.sln"
MSBUILD_PATH = r"C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe"
MSBUILD_ARGS = ["/m", "/p:BuildInParallel=true"]
BINARY_PATH = r"build\Release\bin\codxe.xex"
STAGING_DIR = r"build\staging"
RESOURCES_PATH = r"resources"
VERSION_HEADER_PATH = r"build\Release\obj\git_version.h"


def read_git_version():
    try:
        with open(VERSION_HEADER_PATH, "r") as version_file:
            for line in version_file:
                if line.startswith("#define GIT_VERSION "):
                    git_version = line.split('"')[1]
                    print(f"Git version: {git_version}")
                    return git_version

        raise RuntimeError(f"GIT_VERSION was not found in {VERSION_HEADER_PATH}")
    except Exception as e:
        print(f"An error occurred: {e}")
        exit(1)


# Ensure MSBuild exists
if not os.path.exists(MSBUILD_PATH):
    print(f"ERROR: MSBuild not found at {MSBUILD_PATH}")
    exit(1)

print("Building solution with parallel MSBuild jobs...")
result = subprocess.run([MSBUILD_PATH, SOLUTION_FILE, *MSBUILD_ARGS])
if result.returncode != 0:
    print("ERROR: Build failed.")
    exit(result.returncode)
else:
    print("Build succeeded.")

GIT_VERSION = read_git_version()

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
ZIP_FILE_NAME = f"{PROJECT_NAME}-{GIT_VERSION}.zip"
STAGING_ZIP_PATH = os.path.join("build", ZIP_FILE_NAME)

print("Zipping the staging folder...")
shutil.make_archive(
    base_name=STAGING_ZIP_PATH.replace(".zip", ""), format="zip", root_dir=STAGING_DIR
)
print(f"Staging folder zipped successfully to {STAGING_ZIP_PATH}")
