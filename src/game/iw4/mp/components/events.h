#pragma once

#include "pch.h"

class Events : public Module
{
  public:
    Events();
    ~Events();

    const char *get_name() override
    {
        return "Events";
    };

    static void OnCG_DrawActive(const std::function<void()> &callback);
    static void OnCmdInit(const std::function<void()> &callback);

  private:
    static std::vector<std::function<void()>> cg_drawactive_callbacks;
    static Detour CG_DrawActive_Detour;
    static void CG_DrawActive_Hook(int localClientNum);

    static std::vector<std::function<void()>> cmdinit_callbacks;
    static Detour Cmd_Init_Detour;
    static void Cmd_Init_Hook();
};
