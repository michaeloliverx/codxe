#include "common.h"
#include "plugin_manager.h"

PluginManager::PluginManager()
    : m_should_run(false), m_monitor_thread(nullptr), m_current_title_id(0), m_current_plugin(nullptr)
{
    this->InitConfig();
    // Start the monitoring thread
    xbox::ExCreateThread(&m_monitor_thread,          // out: thread handle
                         0,                          // stack size (0 = default)
                         nullptr,                    // out: thread id (optional)
                         nullptr,                    // api thread startup (reserved; usually nullptr)
                         &PluginManager::ThreadProc, // must be WINAPI (__stdcall)
                         this,                       // LPVOID parameter
                         2                           // creation flags
    );
}

PluginManager::~PluginManager()
{
    // Signal the thread to stop and wait for it to finish
    m_should_run = false;
    if (m_monitor_thread)
    {
        WaitForSingleObject(m_monitor_thread, INFINITE);
        CloseHandle(m_monitor_thread);
        m_monitor_thread = nullptr;
    }
}

void PluginManager::InitConfig()
{
    m_title_id_configs[DASHBOARD] = TitleConfig("Dashboard", nullptr);

    m_title_id_configs[IW2] = TitleConfig("Call of Duty 2",
                                          []() -> Plugin *
                                          {
                                              if (iw2::sp::IW2_SP_Plugin::ShouldLoad())
                                                  return new iw2::sp::IW2_SP_Plugin();
                                              else if (iw2::mp::IW2_MP_Plugin::ShouldLoad())
                                                  return new iw2::mp::IW2_MP_Plugin();
                                              else
                                                  return nullptr;
                                          });

    m_title_id_configs[IW3] = TitleConfig("Call of Duty 4: Modern Warfare",
                                          []() -> Plugin *
                                          {
                                              if (iw3::sp::IW3_SP_Plugin::ShouldLoad())
                                                  return new iw3::sp::IW3_SP_Plugin();
                                              else if (iw3::mp::IW3_MP_Plugin::ShouldLoad())
                                                  return new iw3::mp::IW3_MP_Plugin();
                                              else
                                                  return nullptr;
                                          });

    m_title_id_configs[T4] = TitleConfig("Call of Duty: World at War",
                                         []() -> Plugin *
                                         {
                                             if (t4::sp::T4_SP_Plugin::ShouldLoad())
                                                 return new t4::sp::T4_SP_Plugin();
                                             else if (t4::mp::T4_MP_Plugin::ShouldLoad())
                                                 return new t4::mp::T4_MP_Plugin();
                                             else
                                                 return nullptr;
                                         });

    m_title_id_configs[IW4] = TitleConfig("Call of Duty: Modern Warfare 2",
                                          []() -> Plugin *
                                          {
                                              if (iw4::sp::IW4_SP_Plugin::ShouldLoad())
                                                  return new iw4::sp::IW4_SP_Plugin();
                                              else
                                                  return nullptr;
                                          });

    m_title_id_configs[T5] = TitleConfig("Call of Duty: Black Ops",
                                         []() -> Plugin *
                                         {
                                             if (t5::sp::T5_SP_Plugin::ShouldLoad())
                                                 return new t5::sp::T5_SP_Plugin();
                                             else if (t5::mp::T5_MP_Plugin::ShouldLoad())
                                                 return new t5::mp::T5_MP_Plugin();
                                             else
                                                 return nullptr;
                                         });

    m_title_id_configs[QOS] = TitleConfig("007: Quantum of Solace",
                                          []() -> Plugin *
                                          {
                                              if (qos::sp::QOS_SP_Plugin::ShouldLoad())
                                                  return new qos::sp::QOS_SP_Plugin();
                                              else if (qos::mp::QOS_MP_Plugin::ShouldLoad())
                                                  return new qos::mp::QOS_MP_Plugin();
                                              else
                                                  return nullptr;
                                          });
}

DWORD WINAPI PluginManager::ThreadProc(LPVOID param)
{
    PluginManager *manager = static_cast<PluginManager *>(param);
    manager->m_should_run = true;

    while (manager->m_should_run)
    {
        const uint32_t current_title_id = xbox::XamGetCurrentTitleId();
        if (current_title_id != manager->m_current_title_id)
        {
            manager->OnTitleIDChanged(current_title_id);
        }

        Sleep(50); // Sleep for a while to avoid busy waiting
    }

    return 0;
}

PluginManager::TitleConfig *PluginManager::GetTitleConfig(TitleID title_id)
{
    auto it = m_title_id_configs.find(title_id);
    if (it != m_title_id_configs.end())
    {
        return &it->second;
    }
    return nullptr; // Return nullptr if the title ID is not found
}

void PluginManager::OnTitleIDChanged(uint32_t title_id)
{
    DbgPrint("[PluginManager] Title ID changed to: 0x%08X\n", title_id);

    // If there is a current plugin, clean it up
    if (m_current_plugin)
    {
        DbgPrint("[PluginManager] Cleaning up current plugin for title ID: 0x%08X\n", m_current_title_id);
        delete m_current_plugin;
        m_current_plugin = nullptr;
    }

    // Update the current title ID
    m_current_title_id = title_id;

    Sleep(1000); // Allow some time for the game to load

    // Special case for Dashboard
    if (title_id == NONE || title_id == DASHBOARD)
        return;

    // Initialize the new title's plugins

    PluginManager::TitleConfig *config = GetTitleConfig(static_cast<TitleID>(title_id));
    if (!config)
    {
        DbgPrint("[PluginManager] No configuration found for title ID: 0x%08X\n", title_id);
        return;
    }

    DbgPrint("[PluginManager] Found configuration for title ID: 0x%08X, name: %s\n", title_id, config->name.c_str());

    if (!config->create_plugin)
    {
        DbgPrint("[PluginManager] No plugin factory defined for title ID: 0x%08X\n", title_id);
        return;
    }

    // If the title has a plugin factory, create the plugin
    DbgPrint("[PluginManager] Creating plugin for title ID: 0x%08X\n", title_id);
    const Plugin *plugin = config->create_plugin();
    if (!plugin)
    {
        DbgPrint("[PluginManager] Failed to create plugin for title ID: 0x%08X\n", title_id);
        return;
    }

    // Store the created plugin
    m_current_plugin = plugin;

    DbgPrint("[PluginManager] Plugin created for title ID: 0x%08X\n", title_id);
}
