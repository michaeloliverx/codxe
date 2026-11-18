#include <xtl.h>
#include <xkelib.h>
#include "hook.h"
#include "kernel.h"
#include "notifications.h"
#include "../common/paths.h"
#include "../module/games.h"

namespace
{
const DWORD HARDWARE_FLAG_NOT_RETAIL = 0x10;
ResidentHook finishHook;
char installRoot[MAX_PATH];

struct TitleLaunch
{
    LDR_DATA_TABLE_ENTRY *module;
    PVOID entry;
    const codxe::GameModuleInfo *game;
};
TitleLaunch pendingTitle;

void TitleTerminated()
{
    xbox::notifications::OnTitleTerminate();
    pendingTitle = TitleLaunch();
}
EX_TITLE_TERMINATE_REGISTRATION notification = {reinterpret_cast<PVOID>(TitleTerminated), 0x7C800000, {NULL, NULL}};

BOOL Inactive(const char *message)
{
    xbox::notifications::Initialize(message);
    return TRUE;
}

bool FindRoot(const LDR_DATA_TABLE_ENTRY *self)
{
    return self &&
           codxe::GetModuleDirectory(installRoot, self->FullDllName.Buffer, self->FullDllName.Length / sizeof(WCHAR));
}

void LoadGameModule(const codxe::GameModuleInfo &game)
{
    char directory[MAX_PATH], path[MAX_PATH];
    if (!codxe::JoinPath(directory, installRoot, "modules") || !codxe::JoinPath(path, directory, game.filename))
    {
        DbgPrint("[codxe][loader] Module path is too long: %s\n", game.filename);
        return;
    }
    HANDLE payload = NULL;
    // Loading from TitleEntry gives this image its own title-process CRT and heap state.
    // The kernel unloads title DLLs along with the outgoing executable.
    const NTSTATUS status = XexLoadImage(path, XEX_MODULE_TYPE_TITLE_DLL, 0, &payload);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("[codxe][loader] Could not load %s: %08X\n", path, status);
        return;
    }
    DbgPrint("[codxe][loader] Loaded %s\n", game.filename);
}

DWORD WINAPI TitleEntry(void *parameter)
{
    typedef DWORD(WINAPI * EntryFn)(void *);
    const TitleLaunch title = pendingTitle;
    pendingTitle = TitleLaunch();
    title.module->EntryPoint = title.entry;
    LoadGameModule(*title.game);
    return reinterpret_cast<EntryFn>(title.entry)(parameter);
}

void FinishLoad(LDR_DATA_TABLE_ENTRY *module, const char *commandLine)
{
    typedef void (*XexpFinishExecutableLoad_t)(LDR_DATA_TABLE_ENTRY *, const char *);
    reinterpret_cast<XexpFinishExecutableLoad_t>(finishHook.Original())(module, commandLine);
    pendingTitle = TitleLaunch();
    if (!module)
        return;
    const XEX_EXECUTION_ID *id =
        static_cast<XEX_EXECUTION_ID *>(RtlImageXexHeaderField(module->XexHeaderBase, XEX_HEADER_EXECUTION_ID));
    const codxe::GameModuleInfo *game = id ? codxe::FindGameModule(id->TitleID, module->TimeDateStamp) : NULL;
    if (game)
    {
        pendingTitle.module = module;
        pendingTitle.entry = module->EntryPoint;
        pendingTitle.game = game;
        module->EntryPoint = reinterpret_cast<PVOID>(TitleEntry);
    }
    else if (id)
    {
        DbgPrint("[codxe][loader] No module for title=%08X timestamp=%08X\n", id->TitleID, module->TimeDateStamp);
    }
}
} // namespace

extern "C" BOOL WINAPI CodxeLoaderEntry(HANDLE self, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        if (!FindRoot(static_cast<LDR_DATA_TABLE_ENTRY *>(self)))
            return FALSE;
        const XBOX_KRNL_VERSION version = *XboxKrnlVersion;
        const DWORD hardwareFlags = XboxHardwareInfo->Flags;
        const char *flavour = (hardwareFlags & HARDWARE_FLAG_NOT_RETAIL) ? "development" : "retail";
        DbgPrint("[codxe][loader] Kernel: %u.%u.%u.%u (%s, hardware flags=0x%08X).\n", version.Major, version.Minor,
                 version.Build, version.Qfe, flavour, hardwareFlags);
        xbox::KernelHookTarget target;
        if (!xbox::ResolveFinishLoad(target))
        {
            DbgPrint("[codxe][loader] Kernel load routine not recognized; inactive\n");
            return Inactive("Unsupported kernel - inactive");
        }
        if (!finishHook.Install(target, FinishLoad))
            return Inactive("Kernel hook mismatch - inactive");
        ExRegisterTitleTerminateNotification(&notification, TRUE);
        DbgPrint("[codxe][loader] ready build=%u (%s), hook=%08X root=%s\n", version.Build, flavour, target.address,
                 installRoot);
        xbox::notifications::Initialize();
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        if (finishHook.IsInstalled())
        {
            ExRegisterTitleTerminateNotification(&notification, FALSE);
            // A pending entry wrapper still points into the resident image.
            if (pendingTitle.module)
                pendingTitle.module->EntryPoint = pendingTitle.entry;
            pendingTitle = TitleLaunch();
            finishHook.Remove();
        }
        xbox::notifications::Shutdown();
    }
    return TRUE;
}
