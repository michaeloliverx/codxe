#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class Usermaps : public Module
{
  public:
    Usermaps();

    static void OnDvarInit();
    static void LoadScript(int localClientNum, const char **args);
    static void ApplyScript(int localClientNum, const char **args);
    static void CloseScript(int localClientNum, const char **args);
    static void LoadFilters(int localClientNum, const char **args);
    static void ApplyFilter(int localClientNum, const char **args);
};
} // namespace mp
} // namespace iw3
