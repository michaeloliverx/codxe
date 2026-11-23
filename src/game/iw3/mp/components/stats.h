#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class Stats : public Module
{
  public:
    Stats();

    static void EnsureOfflineStats(int localClientNum);
    static void BeginSplitScreenClassEdit(UiContext *dc);

  private:
    friend class UIScript;

    static void LoadStatsScript(int localClientNum, const char **args);
};
} // namespace mp
} // namespace iw3
