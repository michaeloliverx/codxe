#pragma once

#include "pch.h"

extern const char *const MOD_DIR;
extern const char *const DUMP_DIR;

class Config : public Module
{
  public:
    Config();
    ~Config();

    static bool dump_rawfile;
    static bool dump_map_ents;

    static bool Initialize(HANDLE module);
    static bool MountGameDirectory(const char *game);
    static const char *GetDataRoot();
    static const char *GetActiveMod();
    static std::string ResolveModPath(const char *relativePath);
    static std::string ResolveDataPath(const char *relativePath);

  private:
    static char active_mod[MAX_PATH];

    bool LoadFromJson(const char *jsonBuffer, DWORD bufferSize);
};
