#include "pch.h"
#include "config.h"

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD) - 1)
#endif

const char *CONFIG_PATH = "game:\\_codxe\\codxe.json";
const char *MOD_DIR = "game:\\_codxe\\mods";
const char *DUMP_DIR = "game:\\_codxe\\dump";

// Default values
std::string Config::active_mod = "";
bool Config::dump_rawfile = false;
bool Config::dump_map_ents = false;

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
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file)
        return false;

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    if (size <= 0)
        return false;

    file.seekg(0, std::ios::beg);
    outString.resize(static_cast<size_t>(size));
    return file.read(&outString[0], size).good();
}

Config::Config()
{
    DbgPrint("[codxe][Config] Loading configuration from: %s\n", CONFIG_PATH);

    if (!LoadFromFile(CONFIG_PATH))
    {
        DbgPrint("[codxe][Config] Failed to load config file, using defaults\n");
    }
}

Config::~Config()
{
    // Reset to defaults on cleanup
    active_mod = "";
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

    return true;
}

bool Config::LoadFromFile(const char *path)
{
    std::string jsonContent;
    if (!ReadFileToString(path, jsonContent))
        return false;

    return LoadFromJson(jsonContent.c_str(), static_cast<DWORD>(jsonContent.size()));
}

// Helper method - requires logic beyond simple field access
std::string Config::GetModBasePath()
{
    if (active_mod.empty())
    {
        return "";
    }

    std::string mod_base_path = std::string(MOD_DIR) + "\\" + active_mod;
    if (!DirectoryExists(mod_base_path.c_str()))
    {
        DbgPrint("[codxe][Config] Active mod directory does not exist: %s\n", mod_base_path.c_str());
        return "";
    }
    return mod_base_path;
}
