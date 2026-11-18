#include "pch.h"
#include "games.h"
#include "../common/paths.h"

extern "C" BOOL WINAPI _DllMainCRTStartup(HANDLE, DWORD, LPVOID);

namespace
{
bool StartGame(HANDLE self)
{
    const codxe::GameModuleInfo *game = codxe::FindGameModuleByName(CODXE_STRINGIFY(CODXE_GAME_NAME));
    const auto executable = reinterpret_cast<PLDR_DATA_TABLE_ENTRY>(GetModuleHandle(nullptr));
    const auto execution = executable ? static_cast<XEX_EXECUTION_ID *>(
                                            RtlImageXexHeaderField(executable->XexHeaderBase, XEX_HEADER_EXECUTION_ID))
                                      : nullptr;
    if (!game || !execution || execution->TitleID != game->title || executable->TimeDateStamp != game->timestamp)
    {
        DbgPrint("[codxe][module] Executable does not match %s\n", CODXE_STRINGIFY(CODXE_GAME_NAME));
        return false;
    }

    const xbox::Environment environment = xbox::GetEnvironment();
    // Canary mounts a separate plugin directory for each title.
    const char *dataRoot = "\\Device\\Plugins\\_codxe";
    char xboxDataRoot[MAX_PATH];
    if (environment != xbox::ENVIRONMENT_XENIA)
    {
        const auto module = static_cast<PLDR_DATA_TABLE_ENTRY>(self);
        char root[MAX_PATH];
        if (!module ||
            !codxe::GetModuleDirectory(root, module->FullDllName.Buffer, module->FullDllName.Length / sizeof(WCHAR)))
            return false;
        // This image lives in <install root>/modules/.
        char *slash = strrchr(root, '\\');
        if (!slash || slash == root)
            return false;
        *slash = 0;

        char gamesRoot[MAX_PATH];
        if (!codxe::JoinPath(gamesRoot, root, "games") || !codxe::JoinPath(xboxDataRoot, gamesRoot, game->directory))
            return false;
        dataRoot = xboxDataRoot;
    }

    if (!Config::MountDataRoot(dataRoot))
        return false;

#ifdef CODXE_GAME_IW4_MP_TU6
    if (execution->Version == 0x0000060A)
    {
        execution->Version = 0x0000090A;
        xbox::Notify("Title update spoofed: TU6 -> TU9");
    }
#endif

    // The executable, hooks, workers, CRT statics and allocations share the title lifetime.
    // Title teardown reclaims them together, without calling destructors against a dying game.
    new CODXE_PLUGIN_TYPE();
    DbgPrint("[codxe][%s] Initialized %s, data=%s\n", game->name, GIT_VERSION, dataRoot);
    xbox::Notify(game->description);
    return true;
}
} // namespace

BOOL WINAPI DllMain(HANDLE self, DWORD reason, LPVOID)
{
    return reason != DLL_PROCESS_ATTACH || StartGame(self);
}

// CRT startup initializes the security cookie inside this wrapper.
extern "C" __declspec(safebuffers) BOOL WINAPI CodxeModuleEntry(HANDLE self, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_DETACH)
    {
        DbgPrint("[codxe][%s] Title module detached\n", CODXE_STRINGIFY(CODXE_GAME_NAME));
        return TRUE;
    }
    if (reason == DLL_PROCESS_ATTACH && KeGetCurrentProcessType() != PROC_USER)
    {
        DbgPrint("[codxe][module] Refusing CRT initialization outside title context\n");
        return FALSE;
    }
    return _DllMainCRTStartup(self, reason, reserved);
}
