#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class UIScript : public Module
{
  public:
    UIScript();
    ~UIScript();

  private:
    static bool Run(int localClientNum, const char **args);
    static void UI_RunMenuScript_Hook(int localClientNum, const char **args, const char *actualScript);

    static const UIScriptEntry Scripts[];
    static Detour UI_RunMenuScript_Detour;
};
} // namespace mp
} // namespace iw3
