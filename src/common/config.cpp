#include "pch.h"
#include "config.h"

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD) - 1)
#endif

#ifndef INVALID_FILE_SIZE
#define INVALID_FILE_SIZE ((DWORD) - 1)
#endif

const char *CONFIG_PATH = "game:\\_codxe\\codxe.json";
const char *MOD_DIR = "game:\\_codxe\\mods";
const char *DUMP_DIR = "game:\\_codxe\\dump";

// Default values
char Config::active_mod[MAX_PATH] = {};
bool Config::dump_rawfile = false;
bool Config::dump_map_ents = false;
char Config::mod_base_path[MAX_PATH] = {};
std::string Config::data_root;
bool Config::shared_layout_enabled = false;
std::vector<std::string> Config::mounted_links;

namespace
{
const char *CONFIG_DEVICE_LINK_NAME = "codxe:";
const char *CONFIG_DEVICE_LINK_PATH = "codxe:\\";

const char *GetGameDirectoryName(Config::GameId gameId)
{
    switch (gameId)
    {
    case Config::GAME_T4:
        return "t4";
    case Config::GAME_NONE:
    default:
        return nullptr;
    }
}

bool StartsWith(const std::string &value, const char *prefix)
{
    return strncmp(value.c_str(), prefix, strlen(prefix)) == 0;
}

bool TryGetDeviceRoot(const std::string &path, std::string &device_root)
{
    const char *device_prefix = "\\Device\\";
    if (!StartsWith(path, device_prefix))
        return false;

    size_t root_end = path.find('\\', strlen(device_prefix));
    if (root_end == std::string::npos)
        return false;

    if (path.find("\\Device\\Harddisk") == 0)
    {
        root_end = path.find('\\', root_end + 1);
        if (root_end == std::string::npos)
            return false;
    }

    device_root = path.substr(0, root_end + 1);
    return true;
}

bool MountDevice(const char *link_name, const char *device_path)
{
    const DWORD process_type = KeGetCurrentProcessType();
    const std::string link_path = std::string(process_type == PROC_TYPE_SYSTEM ? "\\System??\\" : "\\??\\") + link_name;

    STRING link = {};
    STRING device = {};
    RtlInitAnsiString(&link, link_path.c_str());
    RtlInitAnsiString(&device, device_path);

    const NTSTATUS delete_status = ObDeleteSymbolicLink(&link);
    const NTSTATUS create_status = ObCreateSymbolicLink(&link, &device);
    UNREFERENCED_PARAMETER(delete_status);

    return NT_SUCCESS(create_status);
}

void UnmountDevice(const char *link_name)
{
    const DWORD process_type = KeGetCurrentProcessType();
    const std::string link_path = std::string(process_type == PROC_TYPE_SYSTEM ? "\\System??\\" : "\\??\\") + link_name;
    STRING link = {};
    RtlInitAnsiString(&link, link_path.c_str());
    ObDeleteSymbolicLink(&link);
}

bool BuildMountedConfigPath(const std::string &device_config_path, std::string &mounted_config_path)
{
    std::string device_path;
    if (!TryGetDeviceRoot(device_config_path, device_path))
        return false;

    if (!MountDevice(CONFIG_DEVICE_LINK_NAME, device_path.c_str()))
        return false;

    mounted_config_path = std::string(CONFIG_DEVICE_LINK_PATH) + device_config_path.substr(device_path.size());
    return true;
}
} // namespace

bool ReadFileToString(const char *path, std::string &outString)
{
    outString.clear();

    const DWORD attrs = GetFileAttributesA(path);
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        DbgPrint("[codxe][Config] GetFileAttributes failed for: %s error=0x%08X\n", path, GetLastError());
    }
    else if ((attrs & FILE_ATTRIBUTE_DIRECTORY) != 0)
    {
        DbgPrint("[codxe][Config] Config path is a directory: %s\n", path);
        return false;
    }

    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        DbgPrint("[codxe][Config] CreateFile failed for: %s error=0x%08X\n", path, GetLastError());
        return false;
    }

    const DWORD size = GetFileSize(file, nullptr);
    if (size == INVALID_FILE_SIZE)
    {
        DbgPrint("[codxe][Config] GetFileSize failed for: %s error=0x%08X\n", path, GetLastError());
        CloseHandle(file);
        return false;
    }

    if (size == 0)
    {
        DbgPrint("[codxe][Config] Config file is empty: %s\n", path);
        CloseHandle(file);
        return false;
    }

    outString.resize(size);

    DWORD bytes_read = 0;
    if (!ReadFile(file, &outString[0], size, &bytes_read, nullptr))
    {
        DbgPrint("[codxe][Config] ReadFile failed for: %s error=0x%08X\n", path, GetLastError());
        CloseHandle(file);
        outString.clear();
        return false;
    }

    CloseHandle(file);

    if (bytes_read != size)
    {
        DbgPrint("[codxe][Config] Short read for: %s expected=%u actual=%u\n", path, size, bytes_read);
        outString.clear();
        return false;
    }

    return true;
}

Config::Config(GameId gameId)
{
    data_root = "game:\\_codxe";
    mounted_links.clear();
    const char *gameDirectoryName = GetGameDirectoryName(gameId);
    shared_layout_enabled = gameDirectoryName != nullptr;

    std::string config_path = CONFIG_PATH;

    if (xbox::GetEnvironment() != xbox::ENVIRONMENT_XENIA)
    {
        const auto module = reinterpret_cast<PLDR_DATA_TABLE_ENTRY>(GetModuleHandle(nullptr));
        if (module != nullptr && module->FullDllName.Buffer != nullptr && module->FullDllName.Length != 0)
        {
            std::string full_path;
            const size_t character_count = module->FullDllName.Length / sizeof(WCHAR);
            full_path.reserve(character_count);
            for (size_t i = 0; i < character_count; ++i)
            {
                full_path.push_back(static_cast<char>(module->FullDllName.Buffer[i]));
            }

            const size_t slash = full_path.find_last_of("\\/");
            if (slash != std::string::npos)
            {
                // On hardware Config is constructed before game: is retargeted to the new title.
                // Only the config file load needs this early executable-relative path.
                config_path = full_path.substr(0, slash) + "\\_codxe\\codxe.json";

                std::string mounted_config_path;
                if (BuildMountedConfigPath(config_path, mounted_config_path))
                {
                    // XDK file APIs do not accept raw \Device\Mass0 paths here. A private symbolic
                    // link keeps the early config load executable-relative without relying on game:.
                    config_path = mounted_config_path;
                    mounted_links.push_back(CONFIG_DEVICE_LINK_NAME);
                }
            }
        }
    }

    if (shared_layout_enabled)
    {
        // Config loads before game: is retargeted on hardware, so inspect the
        // executable-relative path while choosing the single active root.
        const size_t slash = config_path.find_last_of("\\/");
        const std::string local_directory = slash == std::string::npos ? std::string() : config_path.substr(0, slash);
        const std::string nested_local_directory = filesystem::JoinPath(local_directory.c_str(), gameDirectoryName);
        if (filesystem::DirectoryExists(nested_local_directory.c_str()))
        {
            data_root = std::string("game:\\_codxe\\") + gameDirectoryName;
            config_path = filesystem::JoinPath(nested_local_directory.c_str(), "codxe.json");
        }
        else if (!local_directory.empty() && filesystem::DirectoryExists(local_directory.c_str()))
        {
            config_path = filesystem::JoinPath(local_directory.c_str(), "codxe.json");
        }
        else
        {
            data_root.clear();
            if (xbox::GetEnvironment() != xbox::ENVIRONMENT_XENIA)
            {
                // Only the game directory accepts the legacy layout.
                // Device mapping from NXE2GOD:
                // https://github.com/Swizzy/XDK_Projects/blob/f94dcaa93054af745587bcca7ee475ae8130f26c/NXE2GOD/main.cpp#L400-L405
                const char *const link_names[] = {
                    "codxeusb0:", "codxeusb1:", "codxeusb2:", "codxeusb3:", "codxeusb4:", "codxehdd:"};
                const char *const device_paths[] = {"\\Device\\Mass0", "\\Device\\Mass1",
                                                    "\\Device\\Mass2", "\\Device\\Mass3",
                                                    "\\Device\\Mass4", "\\Device\\Harddisk0\\Partition1"};
                for (size_t i = 0; i < ARRAYSIZE(link_names); ++i)
                {
                    if (!MountDevice(link_names[i], device_paths[i]))
                        continue;

                    const std::string root = std::string(link_names[i]) + "\\_codxe\\" + gameDirectoryName;
                    if (!filesystem::DirectoryExists(root.c_str()))
                    {
                        UnmountDevice(link_names[i]);
                        continue;
                    }

                    mounted_links.push_back(link_names[i]);
                    data_root = root;
                    config_path = filesystem::JoinPath(root.c_str(), "codxe.json");
                    break;
                }
            }
        }
    }

    bool loaded = false;
    if (!data_root.empty())
    {
        DbgPrint("[codxe][Config] Selected data root: %s\n", data_root.c_str());
        DbgPrint("[codxe][Config] Loading configuration from: %s\n", config_path.c_str());
        loaded = LoadFromFile(config_path.c_str());
    }

    if (!loaded)
    {
        DbgPrint("[codxe][Config] Failed to load config file, using defaults\n");
    }
}

Config::~Config()
{
    // Reset to defaults on cleanup
    active_mod[0] = '\0';
    mod_base_path[0] = '\0';
    dump_rawfile = false;
    dump_map_ents = false;
    data_root.clear();
    shared_layout_enabled = false;
    for (size_t i = 0; i < mounted_links.size(); ++i)
        UnmountDevice(mounted_links[i].c_str());
    mounted_links.clear();
    DbgPrint("[codxe][Config] Configuration unloaded\n");
}

bool Config::LoadFromJson(const char *jsonBuffer, DWORD bufferSize)
{
    HJSONREADER hJsonReader = XJSONCreateReader();
    if (!hJsonReader)
        return false;

    if (FAILED(XJSONSetBuffer(hJsonReader, jsonBuffer, bufferSize, TRUE)))
    {
        XJSONCloseReader(hJsonReader);
        return false;
    }

    WCHAR valueBuffer[256];
    WCHAR propertyName[128];
    JSONTOKENTYPE jsonTokenType;
    DWORD tokenLength;
    DWORD parsed;

    while (XJSONReadToken(hJsonReader, &jsonTokenType, &tokenLength, &parsed) == S_OK)
    {
        if (jsonTokenType == Json_FieldName)
        {
            XJSONGetTokenValue(hJsonReader, propertyName, ARRAYSIZE(propertyName));

            // Read next token (the value)
            if (XJSONReadToken(hJsonReader, &jsonTokenType, &tokenLength, &parsed) != S_OK)
                continue;

            if (wcscmp(propertyName, L"active_mod") == 0 && jsonTokenType == Json_String)
            {
                XJSONGetTokenValue(hJsonReader, valueBuffer, ARRAYSIZE(valueBuffer));
                wcstombs_s(nullptr, active_mod, ARRAYSIZE(active_mod), valueBuffer, _TRUNCATE);
            }
            else if (wcscmp(propertyName, L"dump_rawfile") == 0)
            {
                dump_rawfile = (jsonTokenType == Json_True);
            }
            else if (wcscmp(propertyName, L"dump_map_ents") == 0)
            {
                dump_map_ents = (jsonTokenType == Json_True);
            }
            else
            {
                DbgPrint("[codxe][Config] WARNING: Ignoring unknown property: %ls\n", propertyName);
            }
        }
    }

    XJSONCloseReader(hJsonReader);

    DbgPrint("[codxe][Config] Configuration loaded:\n");
    DbgPrint("  Active Mod: %s\n", active_mod);
    DbgPrint("  Dump Raw Scripts: %s\n", dump_rawfile ? "true" : "false");
    DbgPrint("  Dump Map Entities: %s\n", dump_map_ents ? "true" : "false");

    if (active_mod[0] != '\0')
    {
        // Config loads before game: is guaranteed to point at the new executable root on hardware.
        // Keep the intended path and let later game-lifecycle file accesses validate it naturally.
        _snprintf_s(mod_base_path, ARRAYSIZE(mod_base_path), _TRUNCATE, "%s\\%s", MOD_DIR, active_mod);
    }

    return true;
}

bool Config::LoadFromFile(const char *path)
{
    std::string jsonContent;
    if (!ReadFileToString(path, jsonContent))
        return false;

    if (!LoadFromJson(jsonContent.c_str(), static_cast<DWORD>(jsonContent.size())))
    {
        DbgPrint("[codxe][Config] Failed to parse config JSON from: %s\n", path);
        return false;
    }

    return true;
}

const char *Config::GetActiveMod()
{
    return active_mod;
}

std::string Config::ResolveModPath(const char *relativePath)
{
    if (!relativePath || !*relativePath || !mod_base_path[0])
        return std::string();

    if (shared_layout_enabled)
    {
        const std::string mod_path = filesystem::JoinPath("mods", active_mod);
        return ResolveDataPath(filesystem::JoinPath(mod_path.c_str(), relativePath).c_str());
    }

    return filesystem::JoinPath(mod_base_path, relativePath);
}

std::string Config::ResolveDataPath(const char *relativePath)
{
    if (!relativePath || !*relativePath || data_root.empty())
        return std::string();

    const std::string path = filesystem::JoinPath(data_root.c_str(), relativePath);
    return filesystem::FileExists(path.c_str()) ? path : std::string();
}

std::string Config::ResolveDataPathForGameFile(const char *relativePath)
{
    std::string path = ResolveDataPath(relativePath);
    if (path.compare(0, 5, "game:") == 0)
        path.replace(0, 5, "D:");
    return path;
}
