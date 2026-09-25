#pragma once

#include "pch.h"

extern const char *CONFIG_PATH;
extern const char *MOD_DIR;
extern const char *DUMP_DIR;

bool ReadFileToString(const char *path, std::string &outString);

class Config : public Module
{
  public:
    enum GameId
    {
        GAME_NONE,
        GAME_T4,
    };

    explicit Config(GameId gameId = GAME_NONE);
    ~Config();

    static bool dump_rawfile;
    static bool dump_map_ents;

    static const char *GetActiveMod();
    static std::string ResolveModPath(const char *relativePath);
    static std::string ResolveDataPath(const char *relativePath);
    static std::string ResolveDataPathForGameFile(const char *relativePath);

  private:
    static char active_mod[MAX_PATH];
    static char mod_base_path[MAX_PATH];
    static std::string data_root;
    static bool shared_layout_enabled;
    static std::vector<std::string> mounted_links;

    bool LoadFromJson(const char *jsonBuffer, DWORD bufferSize);
    bool LoadFromFile(const char *path);
};
