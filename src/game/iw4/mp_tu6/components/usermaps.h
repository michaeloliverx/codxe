#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class Usermaps : public Module
{
  public:
    Usermaps();
    static void OnDvarInit();
    static bool EnsurePreview(const char *map);

  private:
    static int Count();
    static const char *Text(int localClientNum, int index, int column);
    static void Select(int localClientNum, int index);
    static void Load(int localClientNum, const char **args);
    static void Apply(int localClientNum, const char **args);
    static void Close(int localClientNum, const char **args);
};
} // namespace mp_tu6
} // namespace iw4
