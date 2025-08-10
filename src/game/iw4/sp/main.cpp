#include "components/cg.h"
#include "components/clipmap.h"
#include "components/g_client_fields.h"
#include "components/g_scr_main.h"
#include "components/pm.h"
#include "components/scr_parser.h"
#include "main.h"
#include "common.h"

namespace iw4
{
namespace sp
{
void RemoveIdleGunSway()
{
    // BG_CalculateViewMovement_Angles_Idle
    *(volatile uint32_t *)0x82336C9C = 0x60000000;

    // BG_ComputeAndApplyWeaponMovement_IdleAngles
    *(volatile uint32_t *)0x823362A8 = 0x60000000;
}

IW4_SP_Plugin::IW4_SP_Plugin()
{

    // GScr_SetSavedDvar
    // Patches SetSavedDvar SAVED flag check
    *(volatile uint32_t *)0x8221F688 = 0x60000000; // NOP replaces bl Scr_Error

    // GScr_SetDvar_Internal
    *(volatile uint32_t *)0x8220F664 = 0x60000000; // NOP replaces bl Scr_Error
    *(volatile uint32_t *)0x8220F690 = 0x60000000; // NOP replaces bl Scr_Error

    RemoveIdleGunSway();

    DbgPrint("IW4 SP: Registering modules\n");
    RegisterModule(new cg());
    RegisterModule(new clipmap());
    RegisterModule(new g_client_fields());
    RegisterModule(new g_scr_main());
    RegisterModule(new pm());
    RegisterModule(new scr_parser());
}

bool IW4_SP_Plugin::ShouldLoad()
{
    DbgPrint("IW4 SP: Checking if plugin should load\n");
    return (strncmp((char *)0x8203C924, "startSingleplayer", 17) == 0);
}

} // namespace sp
} // namespace iw4
