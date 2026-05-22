#include "pch.h"
#include "plugin_manager.h"

typedef void (*XexpFinishExecutableLoad_t)(PLDR_DATA_TABLE_ENTRY module, const char *commandLine);
static XexpFinishExecutableLoad_t XexpFinishExecutableLoad = reinterpret_cast<XexpFinishExecutableLoad_t>(0x8007AF68);

static PluginManager *g_plugin_manager = nullptr;
static Detour XexpFinishExecutableLoad_Detour;

void XexpFinishExecutableLoad_Hook(PLDR_DATA_TABLE_ENTRY module, const char *commandLine)
{
    XexpFinishExecutableLoad_Detour.GetOriginal<XexpFinishExecutableLoad_t>()(module, commandLine);

    if (g_plugin_manager != nullptr)
    {
        g_plugin_manager->OnExecutableLoaded(module);
    }
}

bool DllMain(HANDLE hModule, DWORD reason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpvReserved);

    if (reason == DLL_PROCESS_ATTACH)
    {
        if (g_plugin_manager != nullptr)
        {
            return TRUE;
        }

        const bool is_xenia = xbox::IsXenia();
        DbgPrint("[codxe] Environment: %s\n", is_xenia ? "Xenia" : "Xbox 360");

        g_plugin_manager = new PluginManager();

        if (is_xenia)
        {
            return TRUE;
        }

        XexpFinishExecutableLoad_Detour = Detour(XexpFinishExecutableLoad, XexpFinishExecutableLoad_Hook);
        if (XexpFinishExecutableLoad_Detour.Install())
        {
            // Preserve the persistent loader hook's trampoline. Game plugins may reset/reuse only trampoline space
            // allocated after this point when unloading on dashboard return.
            g_plugin_manager->SetTrampolinePoolBaseline(Detour::GetTrampolinePoolSize());
        }
        else
        {
            DbgPrint("[codxe] Failed to install XexpFinishExecutableLoad hook.\n");
        }
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        if (!xbox::IsXenia())
        {
            XexpFinishExecutableLoad_Detour.Remove();
        }

        delete g_plugin_manager;
        g_plugin_manager = nullptr;
    }

    return TRUE;
}
