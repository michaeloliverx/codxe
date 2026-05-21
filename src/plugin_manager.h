#pragma once

#include "pch.h"

class PluginManager
{
  public:
    PluginManager();
    ~PluginManager();

    void InitializeForCurrentExecutable();
    void OnExecutableLoaded(PLDR_DATA_TABLE_ENTRY module, const char *commandLine);

  private:
    std::unique_ptr<Plugin> m_current_plugin;
    DWORD m_current_title_id;
    DWORD m_current_timestamp;

    void ResetCurrentPlugin();
};
