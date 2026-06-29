#include "games/iw3_mp/target.h"

#include "games/iw3_mp/addresses.h"
#include "logging/log.h"

namespace codxe
{
namespace games
{
namespace iw3_mp
{
namespace
{

Com_InitDvars_t GetComInitDvars()
{
    return reinterpret_cast<Com_InitDvars_t>(kAddresses.com_init_dvars);
}

Scr_ShutdownSystem_t GetScrShutdownSystem()
{
    return reinterpret_cast<Scr_ShutdownSystem_t>(kAddresses.scr_shutdown_system);
}

CG_DrawActive_t GetCgDrawActive()
{
    return reinterpret_cast<CG_DrawActive_t>(kAddresses.cg_draw_active);
}

} // namespace

bool InstallLifecycleAdapter(PluginContext *ctx)
{
    Com_InitDvars_t com_init_dvars = GetComInitDvars();
    Scr_ShutdownSystem_t scr_shutdown_system = GetScrShutdownSystem();
    CG_DrawActive_t cg_draw_active = GetCgDrawActive();

    if (com_init_dvars == 0 || scr_shutdown_system == 0 || cg_draw_active == 0)
    {
        Log(ctx, LOG_ERROR, "IW3 MP lifecycle adapter has a null engine function");
        return false;
    }

    Log(ctx, LOG_INFO, "IW3 MP lifecycle adapter ready: dvars=0x%08X vm_shutdown=0x%08X draw=0x%08X",
        kAddresses.com_init_dvars, kAddresses.scr_shutdown_system, kAddresses.cg_draw_active);
    return true;
}

} // namespace iw3_mp
} // namespace games
} // namespace codxe
