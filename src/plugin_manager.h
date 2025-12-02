#pragma once

#include "pch.h"

class PluginManager
{
  public:
    PluginManager();
    ~PluginManager();

  private:
    std::unique_ptr<Plugin> m_current_plugin;

    bool m_monitor_active;
    HANDLE m_monitor_thread;

    void OnTitleChanged(DWORD title_id, DWORD timestamp);
    static DWORD WINAPI ThreadProc(LPVOID param);
};
