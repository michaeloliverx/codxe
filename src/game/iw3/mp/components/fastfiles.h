#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class FastFiles : public Module
{
  public:
    FastFiles();
    ~FastFiles();

    static const char *GetUsermapsDirectory();
    static std::string GetUsermapFastfilePath(const char *zoneName);
    static bool CanRestartZones(bool reportError = true);
    static void RestartZones();
    static const std::string &GetLoadedModName();

  private:
    static bool HasModFastfile();
    static std::string GetModFastfilePath();
    static std::string ResolveFastfilePath(const char *filename);

    static Detour CreateFileA_Detour;
    static HANDLE CreateFileA_Hook(const char *filename, DWORD desiredAccess, DWORD shareMode,
                                   LPSECURITY_ATTRIBUTES securityAttributes, DWORD creationDisposition,
                                   DWORD flagsAndAttributes, HANDLE templateFile);

    static Detour DB_LoadXAssets_Detour;
    static void DB_LoadXAssets_Hook(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
};
} // namespace mp
} // namespace iw3
