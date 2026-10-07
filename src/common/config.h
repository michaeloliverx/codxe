#pragma once

#include "pch.h"
#include "game_id.h"

bool ReadFileToString(const char *path, std::string &outString);

class Config : public Module
{
  public:
    explicit Config(GameId::Type gameId, bool allowSharedStorage = false);
    ~Config();

    static bool dump_rawfile;
    static bool dump_map_ents;

    static const char *GetActiveMod();
    static std::string ResolveModPath(const char *relativePath);
    // Build paths within the root selected at startup, including files that do not exist yet.
    static std::string BuildDataPath(const char *relativePath);
    static std::string ResolveDataPath(const char *relativePath);
    static std::string ResolveDataDirectory(const char *relativePath);
    static std::string ResolveDataPathForGameFile(const char *relativePath);

  private:
    static char active_mod[MAX_PATH];
    static std::string data_root;
    static std::vector<std::string> mounted_links;

    bool LoadFromJson(const char *jsonBuffer, DWORD bufferSize);
    bool LoadFromFile(const char *path);
};
