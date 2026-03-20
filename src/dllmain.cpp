#include "pch.h"
#include "plugin_manager.h"

PluginManager *g_plugin_manager = nullptr;

bool DllMain(HANDLE hModule, DWORD reason, LPVOID lpvReserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        if (g_plugin_manager != nullptr)
        {
            DbgPrint("[codxe] DLL_PROCESS_ATTACH: Already initialized, skipping.\n");
            return TRUE;
        }
        DbgPrint("[codxe] DLL_PROCESS_ATTACH: Initializing PluginManager...\n");
        g_plugin_manager = new PluginManager();
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        DbgPrint("[codxe] DLL_PROCESS_DETACH: Cleaning up PluginManager...\n");
        delete g_plugin_manager;
    }

    return TRUE;
}
