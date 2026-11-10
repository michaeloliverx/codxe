#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class UIScript : public Module
{
  public:
    typedef void (*Callback)(int localClientNum, const char **args);
    UIScript();
    ~UIScript();
    static void Add(const char *name, Callback callback);

  private:
    static void Run(int localClientNum, const char **args, const char *actualScript);
    static std::map<std::string, Callback> Scripts;
    static Detour UI_RunMenuScript_Detour;
};
} // namespace mp_tu6
} // namespace iw4
