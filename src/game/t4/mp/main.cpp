#include "common.h"
#include "main.h"
#include "components/branding.h"
#include "components/brush_collision.h"
#include "components/cg.h"
#include "components/gsc_client_fields.h"
#include "components/gsc_client_methods.h"
#include "components/gsc_functions.h"
#include "components/gsc_loader.h"
#include "components/image_loader.h"
#include "components/map.h"
#include "components/test_module.h"
#include "components/ui.h"

namespace t4
{
namespace mp
{
std::vector<Module *> components;

void RegisterComponent(Module *module)
{
    DbgPrint("T4 MP: Component registered: %s\n", module->get_name());
    components.push_back(module);
}

void init()
{
    DbgPrint("T4 MP: Registering modules\n");
    RegisterComponent(new Branding());
    RegisterComponent(new BrushCollision());
    RegisterComponent(new cg());
    RegisterComponent(new GSCClientFields());
    RegisterComponent(new GSCClientMethods());
    RegisterComponent(new GSCFunctions());
    RegisterComponent(new GSCLoader());
    // RegisterComponent(new ImageLoader());
    RegisterComponent(new Map());
    RegisterComponent(new TestModule());
    RegisterComponent(new ui());

    // Patches
    // sub_8220D2D0
    // Patches NO_KNOCKBACK flag check, allows knockback regardless of flags
    *(volatile uint32_t *)0x8220D2E8 = 0x60000000; // NOP replaces bnelr

    // Weapon_RocketLauncher_Fire
    *(volatile uint32_t *)0x8225F98C = 0x60000000;
    *(volatile uint32_t *)0x8225F990 = 0x60000000;
}

void shutdown()
{
    // Clean up in reverse order
    for (auto it = components.rbegin(); it != components.rend(); ++it)
    {
        delete *it;
    }
    components.clear();
}
} // namespace mp
} // namespace t4
