#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class ModList : public Module
{
  public:
    ModList();

    static void OnDvarInit();
    static void OnUIRefresh();

    static std::string GetActiveName();
    static std::string ResolvePath(const char *relativePath);

  private:
    friend class UIScript;

    static std::string GetActivePath();
    static bool RunMod(const char *name);
    static void ClearMods();
    static void QueueModChange(const std::string &name);
    static void RestartMod_f();

    static void ScanMods();
    static void EnsureModsScanned();
    static int GetItemCount();
    static const char *GetItemText(int index);
    static void Select(int index);

    static void LoadModsScript(int localClientNum, const char **args);
    static void RunModScript(int localClientNum, const char **args);
    static void ClearModsScript(int localClientNum, const char **args);

    static std::vector<FeederEntry> Mods;
    static int CurrentMod;
    static bool ModsScanned;
    static dvar_s *fs_game;
    static std::string PendingMod;
    static bool ModChangePending;
    static bool StartupModChecked;
};
} // namespace mp
} // namespace iw3
