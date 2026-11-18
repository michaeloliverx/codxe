#pragma once

#include "pch.h"

extern const char *const MOD_DIR;

class Config : public Module
{
  public:
    Config();

    static bool MountDataRoot(const char *path);
    static const char *GetDataRoot();
    static const char *GetActiveMod();
    static std::string ResolveModPath(const char *relativePath);
    static std::string ResolveDataPath(const char *relativePath);

  private:
    static char active_mod[MAX_PATH];

    bool LoadFromJson(const char *jsonBuffer, DWORD bufferSize);
};
