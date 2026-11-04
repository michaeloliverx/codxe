#include "pch.h"
#include "plugin_manager.h"
#include "notifications.h"
#include "kernel.h"

namespace
{
typedef void (*XexpFinishExecutableLoad_t)(PLDR_DATA_TABLE_ENTRY module, const char *commandLine);

// TODO: make this nicer
const DWORD IW4_MP_TU6_TIMESTAMP = 0x4BE22338;
const DWORD IW4_MP_TU6_EXECUTION_VERSION = 0x0000060A;
const DWORD IW4_MP_TU9_EXECUTION_VERSION = 0x0000090A;

PluginManager *g_plugin_manager = nullptr;
Detour g_xexp_finish_executable_load_detour;
bool g_title_terminate_registered = false;

void TitleTerminateNotification()
{
    xbox::notifications::OnTitleTerminate();
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

        const xbox::KernelProfile *kernel_profile = nullptr;
        if (environment != xbox::ENVIRONMENT_XENIA)
        {
            const XBOX_KRNL_VERSION &version = *XboxKrnlVersion;
            DbgPrint("[codxe] Kernel: %u.%u.%u.%u (hardware flags=0x%08X).\n", version.Major, version.Minor,
                     version.Build, version.Qfe, XboxHardwareInfo->Flags);
            kernel_profile = xbox::FindKernelProfile(version.Build, environment);
            if (kernel_profile == nullptr)
            {
                char message[128];
                _snprintf_s(message, sizeof(message), _TRUNCATE, "Unsupported kernel %u.%u.%u.%u - inactive",
                            version.Major, version.Minor, version.Build, version.Qfe);
                DbgPrint("[codxe] Kernel rejected: no hook mapping for build %u (%s); inactive.\n", version.Build,
                         xbox::GetEnvironmentName(environment));
                xbox::notifications::Initialize(message);
                return TRUE;
            }
            if (!xbox::IsKernelHookValid(*kernel_profile))
            {
                DbgPrint("[codxe] Kernel rejected: hook validation failed; inactive.\n");
                xbox::notifications::Initialize("Kernel hook mismatch - inactive");
                return TRUE;
            }
        }

        g_plugin_manager = new PluginManager();

        if (environment == xbox::ENVIRONMENT_XENIA)
        {
            DbgPrint("[codxe] Xenia: kernel hook checks skipped.\n");
            g_plugin_manager->OnExecutableLoaded(ResolveCurrentTitleId(), ResolveCurrentTimeDateStamp());
            xbox::notifications::Initialize();
            return TRUE;
        }

        ExRegisterTitleTerminateNotification(&g_title_terminate_registration, TRUE);
        g_title_terminate_registered = true;

        const auto hook_address = reinterpret_cast<XexpFinishExecutableLoad_t>(kernel_profile->finish_load_address);

        DbgPrint("[codxe] Installing XexpFinishExecutableLoad hook at %p.\n", hook_address);
        g_xexp_finish_executable_load_detour =
            Detour(reinterpret_cast<void *>(hook_address), XexpFinishExecutableLoad_Hook);

        if (g_xexp_finish_executable_load_detour.Install())
        {
            // Preserve the persistent loader hook's trampoline. Game plugins may reset/reuse only trampoline space
            // allocated after this point when unloading on title changes.
            g_plugin_manager->SetTrampolinePoolBaseline(Detour::GetTrampolinePoolSize());
            DbgPrint("[codxe] XexpFinishExecutableLoad hook installed.\n");
        }
        else
        {
            DbgPrint("[codxe] Failed to install XexpFinishExecutableLoad hook.\n");
        }

        xbox::notifications::Initialize();
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        DbgPrint("[codxe] DLL_PROCESS_DETACH.\n");
        xbox::notifications::Shutdown();

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
