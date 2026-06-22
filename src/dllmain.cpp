#include "pch.h"
#include "plugin_manager.h"

namespace
{
const DWORD XEXP_FINISH_EXECUTABLE_LOAD_RETAIL = 0x8007AF68;
const DWORD XEXP_FINISH_EXECUTABLE_LOAD_DEVKIT = 0x800A17C8;

// TODO: make this nicer
const DWORD IW4_MP_TU6_TIMESTAMP = 0x4BE22338;
const DWORD IW4_MP_TU6_EXECUTION_VERSION = 0x0000060A;
const DWORD IW4_MP_TU9_EXECUTION_VERSION = 0x0000090A;

PluginManager *g_plugin_manager = nullptr;
Detour g_xexp_finish_executable_load_detour;
bool g_title_terminate_registered = false;

void TitleTerminateNotification()
{
    if (g_plugin_manager != nullptr)
    {
        g_plugin_manager->OnTitleTerminate();
    }
}

// The kernel calls this before the outgoing title is fully torn down. Use it to remove title-lifetime hooks/state
// before XexpFinishExecutableLoad observes the next executable.
EX_TITLE_TERMINATE_REGISTRATION g_title_terminate_registration = {
    reinterpret_cast<PVOID>(TitleTerminateNotification),
    0x7C800000,
    {nullptr, nullptr},
};

DWORD ResolveCurrentTitleId()
{
    PXEX_EXECUTION_ID execution_id = nullptr;
    const NTSTATUS status = XamGetExecutionId(&execution_id);
    if (status == 0 && execution_id != nullptr)
    {
        return execution_id->TitleID;
    }

    return XamGetCurrentTitleId();
}

DWORD ResolveCurrentTimeDateStamp()
{
    const auto module = reinterpret_cast<PLDR_DATA_TABLE_ENTRY>(GetModuleHandle(nullptr));
    return module != nullptr ? module->TimeDateStamp : 0;
}

bool SpoofCurrentTitleVersionIfNeeded(DWORD timestamp)
{
    // Report MW2 TU6 as TU9 so spoof-enabled consoles can join each other on Xbox Live.
    if (timestamp != IW4_MP_TU6_TIMESTAMP)
    {
        return false;
    }

    PXEX_EXECUTION_ID execution_id = nullptr;
    const NTSTATUS status = XamGetExecutionId(&execution_id);
    if (status != 0 || execution_id == nullptr)
    {
        return false;
    }

    if (!MmIsAddressValid(execution_id) || !MmIsAddressValid(&execution_id->Version))
    {
        return false;
    }

    if (execution_id->TitleID != TITLE_ID_IW4)
    {
        return false;
    }

    if (execution_id->Version != IW4_MP_TU6_EXECUTION_VERSION)
    {
        return false;
    }

    execution_id->Version = IW4_MP_TU9_EXECUTION_VERSION;
    return true;
}

void XexpFinishExecutableLoad_Hook(PLDR_DATA_TABLE_ENTRY module, const char *commandLine)
{
    if (g_plugin_manager != nullptr)
    {
        g_plugin_manager->OnExecutableLoadStarted();
    }

    g_xexp_finish_executable_load_detour.GetOriginal<XexpFinishExecutableLoad_t>()(module, commandLine);

    if (g_plugin_manager != nullptr)
    {
        const DWORD timestamp = module != nullptr ? module->TimeDateStamp : 0;
        const bool spoofed_title_version = SpoofCurrentTitleVersionIfNeeded(timestamp);
        g_plugin_manager->OnExecutableLoaded(ResolveCurrentTitleId(), timestamp);
        if (spoofed_title_version)
        {
            xbox::Notify("Title update spoofed: TU6 -> TU9");
        }
    }
}
} // namespace

bool DllMain(HANDLE hModule, DWORD reason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpvReserved);

    if (reason == DLL_PROCESS_ATTACH)
    {
        DbgPrint("[codxe] DLL_PROCESS_ATTACH.\n");

        if (g_plugin_manager != nullptr)
        {
            return TRUE;
        }

        const xbox::Environment environment = xbox::GetEnvironment();
        DbgPrint("[codxe] Environment: %s\n", xbox::GetEnvironmentName(environment));

        g_plugin_manager = new PluginManager();

        if (environment == xbox::ENVIRONMENT_XENIA)
        {
            g_plugin_manager->OnExecutableLoaded(ResolveCurrentTitleId(), ResolveCurrentTimeDateStamp());
            return TRUE;
        }

        xbox::ApplySystemPatches();

        ExRegisterTitleTerminateNotification(&g_title_terminate_registration, TRUE);
        g_title_terminate_registered = true;

        const auto hook_address = reinterpret_cast<XexpFinishExecutableLoad_t>(
            environment == xbox::ENVIRONMENT_XBOX_DEVKIT ? XEXP_FINISH_EXECUTABLE_LOAD_DEVKIT
                                                         : XEXP_FINISH_EXECUTABLE_LOAD_RETAIL);

        DbgPrint("[codxe] Installing XexpFinishExecutableLoad hook at %p.\n", hook_address);
        g_xexp_finish_executable_load_detour =
            Detour(reinterpret_cast<void *>(hook_address), XexpFinishExecutableLoad_Hook);

        if (g_xexp_finish_executable_load_detour.Install())
        {
            // Preserve the persistent loader hook's trampoline. Game plugins may reset/reuse only trampoline space
            // allocated after this point when unloading on title changes.
            g_plugin_manager->SetTrampolinePoolBaseline(Detour::GetTrampolinePoolSize());
        }
        else
        {
            DbgPrint("[codxe] Failed to install XexpFinishExecutableLoad hook.\n");
        }
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        DbgPrint("[codxe] DLL_PROCESS_DETACH.\n");

        if (xbox::GetEnvironment() != xbox::ENVIRONMENT_XENIA)
        {
            if (g_title_terminate_registered)
            {
                ExRegisterTitleTerminateNotification(&g_title_terminate_registration, FALSE);
                g_title_terminate_registered = false;
            }

            g_xexp_finish_executable_load_detour.Remove();
        }

        delete g_plugin_manager;
        g_plugin_manager = nullptr;
    }

    return TRUE;
}
