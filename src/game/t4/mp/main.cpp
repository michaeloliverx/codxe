#include "pch.h"
#include "main.h"
#include "components/branding.h"
#include "components/brush_collision.h"
#include "components/events.h"
#include "components/gsc_client_fields.h"
#include "components/gsc_client_methods.h"
#include "components/gsc_functions.h"
#include "components/gsc_loader.h"
#include "components/image_loader.h"
#include "components/map.h"
#include "components/sv_bots.h"
#include "components/ui.h"

namespace t4
{
namespace mp
{

T4_MP_Plugin::T4_MP_Plugin()
{
    RegisterModule(new Config());
    RegisterModule(new Events()); // Must be registered before modules that subscribe to engine events.
    RegisterModule(new Branding());
    RegisterModule(new BrushCollision());
    RegisterModule(new GSCClientFields());
    RegisterModule(new GSCClientMethods());
    RegisterModule(new GSCFunctions());
    RegisterModule(new SVBots());
    RegisterModule(new GSCLoader());
    // RegisterModule(new ImageLoader());
    RegisterModule(new Map());
    RegisterModule(new ui());

    // Patches
    // sub_8220D2D0
    // Patches NO_KNOCKBACK flag check, allows knockback regardless of flags
    *(volatile uint32_t *)0x8220D2E8 = 0x60000000; // NOP replaces bnelr

    // Weapon_RocketLauncher_Fire
    *(volatile uint32_t *)0x8225F98C = 0x60000000;
    *(volatile uint32_t *)0x8225F990 = 0x60000000;
}

T4_MP_Plugin::~T4_MP_Plugin()
{
}

} // namespace mp
} // namespace t4
