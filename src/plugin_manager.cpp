#include "common.h"
#include "plugin_manager.h"

PluginManager::PluginManager() : m_should_run(false), m_monitor_thread(nullptr), m_current_title_id(0)
{
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

        Sleep(1000); // Sleep for a while to avoid busy waiting
    }

    return 0;
}

void PluginManager::OnTitleIDChanged(uint32_t title_id)
{
    DbgPrint("[PluginManager] Title ID changed to: 0x%08X\n", title_id);
    m_current_title_id = title_id;

    switch (title_id)
    {
    case DASHBOARD:
        DbgPrint("[PluginManager] Detected Dashboard title ID.\n");
        break;
    case IW2:
        DbgPrint("[PluginManager] Detected IW2 title ID.\n");
        break;
    case IW3:
        DbgPrint("[PluginManager] Detected IW3 title ID.\n");
        break;
    case T4:
        DbgPrint("[PluginManager] Detected T4 title ID.\n");
        break;
    case IW4:
        DbgPrint("[PluginManager] Detected IW4 title ID.\n");
        break;
    case QOS:
        DbgPrint("[PluginManager] Detected QOS title ID.\n");
        break;
    case T5:
        DbgPrint("[PluginManager] Detected T5 title ID.\n");
        break;
    default:
        DbgPrint("[PluginManager] Unsupported title ID: 0x%08X\n", title_id);
        break;
    }
}
