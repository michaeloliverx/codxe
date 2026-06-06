#pragma once

#include "pch.h"

class console : public Module
{
  public:
    console();
    ~console();

    const char *get_name() override
    {
        return "Console";
    };

  private:
    static Detour SCR_DrawScreenField_Detour;
    static void SCR_DrawScreenField_Hook(int localClientNum, int refreshedUI);

    static void Frame();
};
