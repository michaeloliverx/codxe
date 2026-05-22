#pragma once

#include "pch.h"

class PluginManager
{
  public:
    PluginManager();
    ~PluginManager();

    void InitializeForCurrentExecutable();
    void OnExecutableLoaded(PLDR_DATA_TABLE_ENTRY module);
    void SetTrampolinePoolBaseline(SIZE_T size);

  private:
    std::unique_ptr<Plugin> m_current_plugin;
    SIZE_T m_trampoline_pool_baseline;

    void ResetCurrentPlugin();
};
