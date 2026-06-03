#pragma once

#include "pch.h"

struct GameInfo;

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
