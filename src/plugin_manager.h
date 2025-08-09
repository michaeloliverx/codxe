#pragma once

#include "common.h"

class PluginManager
{
  public:
    enum TitleID : uint32_t
    {
        DASHBOARD = 0xFFFE07D1,
        IW2 = 0x415607D1,
        IW3 = 0x415607E6,
        T4 = 0x4156081C,
        IW4 = 0x41560817,
        QOS = 0x415607FF,
        T5 = 0x41560855,
    };

    PluginManager();
    ~PluginManager();

  private:
    void OnTitleIDChanged(uint32_t title_id);
    static DWORD WINAPI ThreadProc(LPVOID param);

    bool m_should_run;
    HANDLE m_monitor_thread;
    uint32_t m_current_title_id;
};
