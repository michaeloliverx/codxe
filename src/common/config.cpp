#include "config.h"

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD) - 1)
#endif

const char *CONFIG_PATH = "game:\\_codxe\\codxe.json";
const char *MOD_DIR = "game:\\_codxe\\mods";
const char *DUMP_DIR = "game:\\_codxe\\dump";

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

void DbgPrintConfig(const Config &config)
{
    DbgPrint("Configuration:\n");
    DbgPrint("Active Mod: %s\n", config.active_mod.c_str());
    DbgPrint("Dump Raw Scripts: %s\n", config.dump_raw ? "true" : "false");
    DbgPrint("Dump Map Entities: %s\n", config.dump_map_ents ? "true" : "false");
}

bool LoadConfigFromJson(const char *jsonBuffer, DWORD bufferSize, Config &outConfig)
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
                outConfig.active_mod = narrowValue;
            }
            else if (wcscmp(propertyName, L"dump_raw") == 0)
            {
                outConfig.dump_raw = (jsonTokenType == Json_True);
            }
            else if (wcscmp(propertyName, L"dump_map_ents") == 0)
            {
                outConfig.dump_map_ents = (jsonTokenType == Json_True);
            }
            else
            {
                DbgPrint("WARNING: Ignoring unknown property in config: %ls\n", propertyName);
            }
        }
    }

    XJSONCloseReader(hJsonReader);

    DbgPrintConfig(outConfig);

    return true;
}

bool LoadConfigFromFile(const char *path, Config &outConfig)
{
    std::string jsonContent;
    if (!ReadFileToString(path, jsonContent))
        return false;

    return LoadConfigFromJson(jsonContent.c_str(), static_cast<DWORD>(jsonContent.size()), outConfig);
}
