#pragma once

#include "pch.h"

extern const char *CONFIG_PATH;
extern const char *MOD_DIR;
extern const char *DUMP_DIR;

bool DirectoryExists(const char *path);
bool FileExists(const char *path);
bool ReadFileToString(const char *path, std::string &outString);

class Config : public Module
{
  public:
    Config();
    ~Config();

    const char *get_name() override
    {
        return "Config";
    }

    static std::string active_mod;
    static bool dump_rawfile;
    static bool dump_map_ents;

    static std::string GetModBasePath();

  private:
    bool LoadFromJson(const char *jsonBuffer, DWORD bufferSize);
    bool LoadFromFile(const char *path);
};
