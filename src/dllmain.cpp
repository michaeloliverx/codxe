#include "core/runtime.h"
#include "hooks/detour.h"
#include "logging/log.h"
#include "platform/xbox360.h"

#include <xtl.h>

namespace
{

struct LoaderState
{
    codxe::PluginContext context;
    codxe::Detour xexp_finish_executable_load_detour;
    bool runtime_initialized;
    bool loader_hook_installed;
};

LoaderState g_loader;

void XexpFinishExecutableLoad_Hook(codxe::LoaderDataTableEntry *module, const char *command_line)
{
    codxe::RuntimeUnloadTarget(&g_loader.context);

    codxe::XexpFinishExecutableLoad_t original =
        codxe::DetourGetOriginal<codxe::XexpFinishExecutableLoad_t>(&g_loader.xexp_finish_executable_load_detour);
    if (original != 0)
        original(module, command_line);

    codxe::RuntimeLoadExecutable(&g_loader.context, codxe::GetCurrentTitleId(), codxe::GetExecutableTimestamp(module));
}

bool InstallLoaderHook()
{
    codxe::XexpFinishExecutableLoad_t hook_address = codxe::GetXexpFinishExecutableLoad(g_loader.context.platform.kind);
    if (hook_address == 0)
    {
        codxe::Log(&g_loader.context, codxe::LOG_ERROR, "XexpFinishExecutableLoad hook address unavailable");
        return false;
    }

    codxe::Log(&g_loader.context, codxe::LOG_INFO, "installing XexpFinishExecutableLoad hook at 0x%08X",
               reinterpret_cast<uint32_t>(hook_address));

    codxe::DetourInit(&g_loader.xexp_finish_executable_load_detour, reinterpret_cast<void *>(hook_address),
                      XexpFinishExecutableLoad_Hook);
    g_loader.loader_hook_installed = codxe::DetourInstall(&g_loader.xexp_finish_executable_load_detour);

    if (!g_loader.loader_hook_installed)
        codxe::Log(&g_loader.context, codxe::LOG_ERROR, "failed to install XexpFinishExecutableLoad hook");

    return g_loader.loader_hook_installed;
}

} // namespace

BOOL WINAPI DllMain(HANDLE module_handle, DWORD reason, LPVOID reserved)
{
    (void)module_handle;
    (void)reserved;

    if (reason == DLL_PROCESS_ATTACH)
    {
        if (g_loader.runtime_initialized)
            return TRUE;

        g_loader.runtime_initialized = codxe::RuntimeInit(&g_loader.context);
        if (!g_loader.runtime_initialized)
            return TRUE;

        if (g_loader.context.platform.kind == codxe::PLATFORM_XENIA)
        {
            codxe::RuntimeLoadExecutable(&g_loader.context, codxe::GetCurrentTitleId(),
                                         codxe::GetCurrentExecutableTimestamp());
            return TRUE;
        }

        InstallLoaderHook();
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        if (g_loader.runtime_initialized)
        {
            codxe::RuntimeShutdown(&g_loader.context);

            if (g_loader.loader_hook_installed)
            {
                codxe::DetourRemove(&g_loader.xexp_finish_executable_load_detour);
                g_loader.loader_hook_installed = false;
            }

            g_loader.runtime_initialized = false;
        }
    }

    return TRUE;
}
