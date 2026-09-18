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
    static void ReloadModZone();

  private:
    static bool HasModFastfile();
    static std::string GetModFastfilePath();

    static Detour DB_BuildOSPath_Detour;
    static int DB_BuildOSPath_Hook(const char *zoneName, unsigned int size, char *filename);

    static Detour DB_LoadXAssets_Detour;
    static void DB_LoadXAssets_Hook(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
};
} // namespace mp
} // namespace iw3
