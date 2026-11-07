#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{
class stats : public Module
{
  public:
    stats();
    ~stats();

    static void OnCmdInit();
    static void LoadOfflineStats(int localClientNum);
    static void RestoreLiveStats();
};
} // namespace mp
} // namespace t4
