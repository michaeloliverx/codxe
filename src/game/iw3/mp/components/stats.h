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
    ~Stats();

    static void EnsureOfflineStats(int localClientNum);

  private:
    friend class UIScript;

    static void LoadStatsScript(int localClientNum, const char **args);
};
} // namespace mp
} // namespace iw3
