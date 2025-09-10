#pragma once

#include "common.h"

class Console : public Module
{

  public:
    struct Settings
    {
        float text_color[4];
        float background_color[4];
        float input_text_color[4];
        float prompt_color[4];
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

    static Detour CL_ConsolePrint_Detour;
    static void CL_ConsolePrint_Hook(int localClientNum, int channel, const char *txt, unsigned int duration,
                                     unsigned int pixelWidth, int flags);

    static void RenderConsole();
    static void HandleInput();

    static void Open();
    static void Close();
    static void Toggle();
    static bool IsOpen();
    static void ExecuteCommand(const char *command);
};
