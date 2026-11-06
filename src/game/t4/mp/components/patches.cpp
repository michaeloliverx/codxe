#include "pch.h"
#include "patches.h"

namespace t4
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
    const unsigned int size = sizeof(client_t) * T4_MAX_CLIENTS;
    *svs_clients = reinterpret_cast<client_t *>(PMem_Alloc(size, __alignof(client_t), PAGE_READWRITE, PHYS_ALLOC_HIGH));
    memset(*svs_clients, 0, size);
}

int SV_SpawnServer_Hook(const char *server)
{
    // Apply this after default_splitscreen.cfg and before SV_Startup sizes the snapshot ring.
    if (Dvar_GetBool("splitscreen"))
        Dvar_SetIntByName("sv_maxclients", T4_MAX_CLIENTS);

    return SV_SpawnServer_Detour.GetOriginal<decltype(SV_SpawnServer)>()(server);
}
} // namespace

void DisableFastfileRsaChecks()
{
    // DBX_AuthLoad_ValidateHash
    ppc::Nop(0x821D7E9C);
    ppc::Nop(0x821D7F94);
    ppc::Nop(0x821D7FE4);

    // DBX_AuthLoad_ValidateSignature
    ppc::Nop(0x821D7EB4);
}

void EnableExplosiveKnockback()
{
    // sub_8220D2D0
    // Patches NO_KNOCKBACK flag check, allowing knockback regardless of flags.
    ppc::Nop(0x8220D2E8); // NOP replaces bnelr
}

void EnableRocketJumping()
{
    // Weapon_RocketLauncher_Fire
    ppc::Nop(0x8225F98C);
    ppc::Nop(0x8225F990);
}

Patches::Patches()
{
    DisableFastfileRsaChecks();
    EnableExplosiveKnockback();
    EnableRocketJumping();

    SV_AllocateClientMemory_Detour = Detour(SV_AllocateClientMemory, SV_AllocateClientMemory_Hook);
    SV_AllocateClientMemory_Detour.Install();

    SV_SpawnServer_Detour = Detour(SV_SpawnServer, SV_SpawnServer_Hook);
    SV_SpawnServer_Detour.Install();

    // default loc_warnings off to prevent console spam
    *(volatile uint8_t *)0x82292ABF = 0x0;
}

Patches::~Patches()
{
    SV_SpawnServer_Detour.Remove();
    SV_AllocateClientMemory_Detour.Remove();
}
} // namespace mp
} // namespace t4
