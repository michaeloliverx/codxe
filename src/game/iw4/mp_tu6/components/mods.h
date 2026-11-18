#pragma once
#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class ModList : public Module
{
  public:
    ModList();
    static void OnDvarInit();
    static std::string GetActiveName();
    static std::string ResolvePath(const char *relativePath);
};
} // namespace mp_tu6
} // namespace iw4
