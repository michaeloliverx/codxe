#include "pch.h"
#include "components/branding.h"
#include "components/events.h"
#include "components/patches.h"
#include "components/pm.h"
#include "main.h"

namespace iw4
{
namespace mp
{

IW4_MP_Plugin::IW4_MP_Plugin()
{
    RegisterModule(new Events());
    RegisterModule(new Branding());
    RegisterModule(new patches());
    RegisterModule(new pm());
}

} // namespace mp
} // namespace iw4
