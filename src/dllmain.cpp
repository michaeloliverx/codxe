#include "common.h"
#include "plugin_manager.h"

PluginManager *g_plugin_manager = nullptr;

bool DllMain(HANDLE hModule, DWORD reason, LPVOID lpvReserved)
{
    if (reason == DLL_PROCESS_ATTACH)
        g_plugin_manager = new PluginManager();
    else if (reason == DLL_PROCESS_DETACH)
        delete g_plugin_manager;

    return TRUE;
}
