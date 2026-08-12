#include "pch.h"
#include "components/cg.h"
#include "components/clipmap.h"
#include "components/gsc_fields.h"
#include "components/gsc.h"
#include "components/pm.h"
#include "components/scr_parser.h"
#include "main.h"

namespace iw4
{
namespace sp
{
void RemoveIdleGunSway()
{
    // BG_CalculateViewMovement_Angles_Idle
    ppc::Nop(0x82336C9C);

    // BG_ComputeAndApplyWeaponMovement_IdleAngles
    ppc::Nop(0x823362A8);
}

IW4_SP_Plugin::IW4_SP_Plugin()
{

    // GScr_SetSavedDvar
    // Patches SetSavedDvar SAVED flag check
    ppc::Nop(0x8221F688); // NOP replaces bl Scr_Error

    // GScr_SetDvar_Internal
    ppc::Nop(0x8220F664); // NOP replaces bl Scr_Error
    ppc::Nop(0x8220F690); // NOP replaces bl Scr_Error

    RemoveIdleGunSway();

    RegisterModule(new Config());
    RegisterModule(new cg());
    RegisterModule(new clipmap());
    RegisterModule(new GSCFields());
    RegisterModule(new GSC());
    RegisterModule(new pm());
    RegisterModule(new scr_parser());
}

} // namespace sp
} // namespace iw4
