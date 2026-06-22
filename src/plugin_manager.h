#pragma once

#include "pch.h"

struct GameInfo;

enum TitleID : DWORD
{
    TITLE_ID_DASHBOARD = 0xFFFE07D1,
    TITLE_ID_IW2 = 0x415607D1,
    TITLE_ID_IW3 = 0x415607E6,
    TITLE_ID_IW4 = 0x41560817,
    TITLE_ID_IW5 = 0x415608CB,
    TITLE_ID_QOS = 0x415607FF,
    TITLE_ID_T4 = 0x4156081C,
    TITLE_ID_T5 = 0x41560855,
};

class PluginManager
{
  public:
    PluginManager();
    ~PluginManager();

    void OnExecutableLoadStarted();
    void OnExecutableLoaded(DWORD title_id, DWORD timestamp);
    void OnTitleTerminate();
    void SetTrampolinePoolBaseline(SIZE_T size);

  private:
    std::unique_ptr<Plugin> m_current_plugin;
    SIZE_T m_trampoline_pool_baseline;

    bool LoadPlugin(const GameInfo *info);
    void ResetCurrentPlugin();
};
