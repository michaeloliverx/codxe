#pragma once

#include "pch.h"

namespace qos
{
namespace mp
{

class Events : public Module
{
  public:
    Events();
    ~Events();

  private:
    static Detour CG_DrawActive_Detour;
    static void CG_DrawActive_Hook(int localClientNum);

    static Detour CG_Init_Detour;
    static void CG_Init_Hook(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum);
};
} // namespace mp
} // namespace qos