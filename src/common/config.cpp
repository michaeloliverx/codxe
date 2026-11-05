#include "pch.h"
#include "config.h"

const char *const MOD_DIR = "codxe:\\mods";
const char *const DUMP_DIR = "codxe:\\dump";

// Default values
char Config::active_mod[MAX_PATH] = {};
bool Config::dump_rawfile = false;
bool Config::dump_map_ents = false;

namespace
{
const char *const CONFIG_PATH = "codxe:\\codxe.json";
char installRoot[MAX_PATH];

bool MountDevice(const char *name, const char *path)
{
    STRING link, device;
    RtlInitAnsiString(&link, name);
    RtlInitAnsiString(&device, path);
    ObDeleteSymbolicLink(&link);
    const NTSTATUS status = ObCreateSymbolicLink(&link, &device);
    if (!NT_SUCCESS(status))
        DbgPrint("[codxe][Config] Could not mount %s at %s: %08X\n", path, name, status);
    return NT_SUCCESS(status);
}

bool ReadFileToString(const char *path, std::string &outString)
{
    outString.clear();

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

} // namespace

bool Config::Initialize(HANDLE module)
{
    if (xbox::GetEnvironment() == xbox::ENVIRONMENT_XENIA)
    {
        // Canary mounts the current title's plugin folder and omits FullDllName.
        strcpy_s(installRoot, "\\Device\\Plugins\\_codxe");
    }
    else
    {
        const auto self = static_cast<PLDR_DATA_TABLE_ENTRY>(module);
        const unsigned length = self->FullDllName.Length / sizeof(WCHAR);
        if (length >= ARRAYSIZE(installRoot))
            return false;
        unsigned lastSlash = 0;
        for (unsigned i = 0; i < length; ++i)
        {
            const WCHAR c = self->FullDllName.Buffer[i];
            if (c > 127)
                return false;
            installRoot[i] = static_cast<char>(c);
            if (c == '\\')
                lastSlash = i;
        }
        if (!lastSlash)
            return false;
        installRoot[lastSlash] = 0;
    }
    DbgPrint("[codxe][Config] Install root: %s\n", installRoot);
    return true;
}

bool Config::MountGameDirectory(const char *game)
{
    char path[MAX_PATH];
    if (_snprintf_s(path, ARRAYSIZE(path), _TRUNCATE, "%s\\games\\%s", installRoot, game) < 0)
    {
        DbgPrint("[codxe][Config] Data path is too long for %s\n", game);
        return false;
    }
    if (!MountDevice("\\??\\codxe:", path))
        return false;
    // The current loader constructs components in system context; game I/O uses title context.
    if (xbox::GetEnvironment() != xbox::ENVIRONMENT_XENIA && !MountDevice("\\System??\\codxe:", path))
        return false;
    DbgPrint("[codxe][Config] Game data: %s\n", path);
    return true;
}

const char *Config::GetDataRoot()
{
    return "codxe:\\";
}

Config::Config()
{
    DbgPrint("[codxe][Config] Loading configuration from: %s\n", CONFIG_PATH);

    std::string contents;
    if (!ReadFileToString(CONFIG_PATH, contents) ||
        !LoadFromJson(contents.c_str(), static_cast<DWORD>(contents.size())))
    {
        DbgPrint("[codxe][Config] Failed to load config file, using defaults\n");
    }
}

Config::~Config()
{
    // Reset to defaults on cleanup
    active_mod[0] = '\0';
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

    return true;
}

const char *Config::GetActiveMod()
{
    return active_mod;
}

std::string Config::ResolveModPath(const char *relativePath)
{
    if (!relativePath || !*relativePath || !active_mod[0])
        return std::string();

    const std::string directory = filesystem::JoinPath(MOD_DIR, active_mod);
    return filesystem::JoinPath(directory.c_str(), relativePath);
}

std::string Config::ResolveDataPath(const char *relativePath)
{
    if (!relativePath || !*relativePath)
        return std::string();

    const std::string path = filesystem::JoinPath(GetDataRoot(), relativePath);
    return filesystem::FileExists(path.c_str()) ? path : std::string();
}
