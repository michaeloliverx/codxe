#include "pch.h"
#include "patches.h"

namespace iw3
{
namespace mp
{
namespace
{
Detour SV_AllocateClientMemory_Detour;
Detour SV_SpawnServer_Detour;

void SV_AllocateClientMemory_Hook()
{
    // Splitscreen normally allocates only four server slots, independently of sv_maxclients.
    const unsigned int size = sizeof(client_t) * MAX_CLIENTS;
    svs->clients = reinterpret_cast<client_t *>(PMem_Alloc(size, __alignof(client_t), PAGE_READWRITE, PHYS_ALLOC_HIGH));
    memset(svs->clients, 0, size);
}

void SV_SpawnServer_Hook(const char *server)
{
    // Apply this after default_splitscreen.cfg and before SV_Startup sizes the snapshot ring.
    if (Dvar_GetBool("splitscreen"))
        Dvar_SetIntByName("sv_maxclients", MAX_CLIENTS);

    SV_SpawnServer_Detour.GetOriginal<decltype(SV_SpawnServer)>()(server);
}

void ExpandPMem()
{
    // Xenia has its own TOML patch that has a higher limit
    // Maybe we should do it at runtime here as well :shrug:
    if (xbox::GetEnvironment() == xbox::ENVIRONMENT_XENIA)
        return;

    // Increase memory allocation by 10 MiB which gives room for larger fastfiles.
    // PMem_Init: 0x19700000 + 10 MiB = 0x1A100000.
    // Keep the allocation request and the retry-size reference in sync.
    *(volatile uint16_t *)0x821CFD1A = 0x1A10;
    *(volatile uint16_t *)0x821CFD2A = 0x1A10;
}
} // namespace

Patches::Patches()
{
    ExpandPMem();

    SV_AllocateClientMemory_Detour = Detour(SV_AllocateClientMemory, SV_AllocateClientMemory_Hook);
    SV_AllocateClientMemory_Detour.Install();

    SV_SpawnServer_Detour = Detour(SV_SpawnServer, SV_SpawnServer_Hook);
    SV_SpawnServer_Detour.Install();
}

Patches::~Patches()
{
    SV_SpawnServer_Detour.Remove();
    SV_AllocateClientMemory_Detour.Remove();
}
} // namespace mp
} // namespace iw3
