#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{
class Usermaps : public Module
{
  public:
    Usermaps();

    static const int FEEDER_ID = 60;
    static void OnDvarInit();
    static void OnUIInit();
    static void OnUIRefresh();
    static bool RunMenuScript(int localClientNum, const char *command);
    static int Count();
    static const char *Text(int index);
    static void Select(int index);
};
} // namespace mp
} // namespace t4
