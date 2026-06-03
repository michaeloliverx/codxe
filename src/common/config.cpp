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
std::string Config::active_mod = "";
bool Config::dump_rawfile = false;
bool Config::dump_map_ents = false;
std::string Config::mod_base_path = "";

namespace
{
const char *CONFIG_DEVICE_LINK_NAME = "codxe:";
const char *CONFIG_DEVICE_LINK_PATH = "codxe:\\";

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

bool MountConfigDevice(const char *device_path)
{
    const DWORD process_type = KeGetCurrentProcessType();
    const std::string link_path =
        std::string(process_type == PROC_TYPE_SYSTEM ? "\\System??\\" : "\\??\\") + CONFIG_DEVICE_LINK_NAME;

    STRING link = {};
    STRING device = {};
    RtlInitAnsiString(&link, link_path.c_str());
    RtlInitAnsiString(&device, device_path);

    const NTSTATUS delete_status = ObDeleteSymbolicLink(&link);
    const NTSTATUS create_status = ObCreateSymbolicLink(&link, &device);
    UNREFERENCED_PARAMETER(delete_status);

    return NT_SUCCESS(create_status);
}

bool BuildMountedConfigPath(const std::string &device_config_path, std::string &mounted_config_path)
{
    std::string device_path;
    if (!TryGetDeviceRoot(device_config_path, device_path))
        return false;

    if (!MountConfigDevice(device_path.c_str()))
        return false;

    mounted_config_path = std::string(CONFIG_DEVICE_LINK_PATH) + device_config_path.substr(device_path.size());
    return true;
}
} // namespace

bool DirectoryExists(const char *path)
{
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
}

bool FileExists(const char *path)
{
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
}

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

Config::Config()
{
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
                }
            }
        }
    }

    DbgPrint("[codxe][Config] Loading configuration from: %s\n", config_path.c_str());

    if (!LoadFromFile(config_path.c_str()))
    {
        DbgPrint("[codxe][Config] Failed to load config file, using defaults\n");
    }
}

Config::~Config()
{
    // Reset to defaults on cleanup
    active_mod = "";
    mod_base_path = "";
    dump_rawfile = false;
    dump_map_ents = false;
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
                char narrowValue[256];
                wcstombs(narrowValue, valueBuffer, sizeof(narrowValue));
                active_mod = narrowValue;
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
    DbgPrint("  Active Mod: %s\n", active_mod.c_str());
    DbgPrint("  Dump Raw Scripts: %s\n", dump_rawfile ? "true" : "false");
    DbgPrint("  Dump Map Entities: %s\n", dump_map_ents ? "true" : "false");

    if (!active_mod.empty())
    {
        // Config loads before game: is guaranteed to point at the new executable root on hardware.
        // Keep the intended path and let later game-lifecycle file accesses validate it naturally.
        mod_base_path = std::string(MOD_DIR) + "\\" + active_mod;
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

std::string Config::GetModBasePath()
{
    return mod_base_path;
}

const char *Config::GetModBasePathCStr()
{
    return mod_base_path.c_str();
}
