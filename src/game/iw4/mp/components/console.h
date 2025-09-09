#pragma once

#include "common.h"

class Console : public Module
{

  public:
    struct Settings
    {
        float text_color[4];
        float background_color[4];
    };

    Console();
    ~Console();

    const char *get_name() override
    {
        return "Console";
    };

  private:
    static Detour SCR_DrawScreenField_Detour;
    static void SCR_DrawScreenField_Hook(int localClientNum, int refreshedUI);
    static void RenderConsole();
};
