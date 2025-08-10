#pragma once

#include "common.h"
#include <unordered_map>

class PluginManager
{
  public:
    enum TitleID : uint32_t
    {
        NONE = 0,
        DASHBOARD = 0xFFFE07D1,
        IW2 = 0x415607D1,
        IW3 = 0x415607E6,
        T4 = 0x4156081C,
        IW4 = 0x41560817,
        QOS = 0x415607FF,
        T5 = 0x41560855,
    };

    struct TitleConfig
    {
        std::string name;
        std::function<Plugin *()> create_plugin;

        TitleConfig() {}; // Default constructor for unordered_map

        TitleConfig(const std::string &title_name, std::function<Plugin *()> create_plugin)
            : name(title_name), create_plugin(create_plugin)
        {
        }
    };

    std::unordered_map<TitleID, TitleConfig> m_title_id_configs;

    PluginManager();
    ~PluginManager();

  private:
    void InitConfig();
    TitleConfig *GetTitleConfig(TitleID title_id);
    bool m_should_run;
    HANDLE m_monitor_thread;
    uint32_t m_current_title_id;
    const Plugin *m_current_plugin;
    void OnTitleIDChanged(uint32_t title_id);
    static DWORD WINAPI ThreadProc(LPVOID param);
};
