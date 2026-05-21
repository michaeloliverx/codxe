#include "pch.h"
#include "plugin_manager.h"

typedef void (*XexpFinishExecutableLoad_t)(PLDR_DATA_TABLE_ENTRY module, const char *commandLine);
static XexpFinishExecutableLoad_t XexpFinishExecutableLoad = reinterpret_cast<XexpFinishExecutableLoad_t>(0x8009F340);

static PluginManager *g_plugin_manager = nullptr;
static Detour XexpFinishExecutableLoad_Detour;

void XexpFinishExecutableLoad_Hook(PLDR_DATA_TABLE_ENTRY module, const char *commandLine)
{
    if (g_plugin_manager != nullptr)
    {
        g_plugin_manager->OnExecutableLoaded(module, commandLine);
    }

    XexpFinishExecutableLoad_Detour.GetOriginal<XexpFinishExecutableLoad_t>()(module, commandLine);
}

bool DllMain(HANDLE hModule, DWORD reason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpvReserved);

    if (reason == DLL_PROCESS_ATTACH)
    {
        if (g_plugin_manager != nullptr)
        {
            DbgPrint("[codxe] DLL_PROCESS_ATTACH: PluginManager already initialized.\n");
            return TRUE;
        }

        DbgPrint("[codxe] DLL_PROCESS_ATTACH: Creating PluginManager.\n");
        g_plugin_manager = new PluginManager();

        if (xbox::IsXenia())
        {
            DbgPrint("[codxe] DLL_PROCESS_ATTACH: Xenia detected, skipping XexpFinishExecutableLoad hook.\n");
            return TRUE;
        }

        DbgPrint("[codxe] DLL_PROCESS_ATTACH: Installing XexpFinishExecutableLoad hook.\n");
        XexpFinishExecutableLoad_Detour = Detour(XexpFinishExecutableLoad, XexpFinishExecutableLoad_Hook);
        if (!XexpFinishExecutableLoad_Detour.Install())
        {
            DbgPrint("[codxe] DLL_PROCESS_ATTACH: Failed to install XexpFinishExecutableLoad hook.\n");
        }
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        DbgPrint("[codxe] DLL_PROCESS_DETACH: Shutting down.\n");

        if (!xbox::IsXenia())
        {
            XexpFinishExecutableLoad_Detour.Remove();
        }

        delete g_plugin_manager;
        g_plugin_manager = nullptr;
    }

    return TRUE;
}
