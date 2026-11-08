#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{
class ModList : public Module
{
  public:
    ModList();
    ~ModList();

    static void OnDvarInit();
    static bool RunMenuScript(int localClientNum, const char *command);
    static std::string GetActiveName();
    static std::string ResolvePath(const char *relativePath);
};
} // namespace mp
} // namespace t4
