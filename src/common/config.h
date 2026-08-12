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

    static bool dump_rawfile;
    static bool dump_map_ents;

    static const char *GetActiveMod();
    static const char *GetModBasePath();

  private:
    static char active_mod[MAX_PATH];
    static char mod_base_path[MAX_PATH];

    bool LoadFromJson(const char *jsonBuffer, DWORD bufferSize);
    bool LoadFromFile(const char *path);
};
