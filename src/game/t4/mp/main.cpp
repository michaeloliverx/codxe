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
#include "components/patches.h"
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
    RegisterModule(new Patches());
    RegisterModule(new ui());
}

T4_MP_Plugin::~T4_MP_Plugin()
{
}

} // namespace mp
} // namespace t4
