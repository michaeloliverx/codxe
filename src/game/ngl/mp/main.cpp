#include "pch.h"
#include "main.h"
#include "components/pm.h"

namespace ngl
{
namespace mp
{

NGL_MP_Plugin::NGL_MP_Plugin()
{
    DbgPrint("NGL MP Plugin initialized\n");

    RegisterModule(new PlayerMovement());
}

NGL_MP_Plugin::~NGL_MP_Plugin()
{
    DbgPrint("NGL MP Plugin shutting down\n");
}

} // namespace mp
} // namespace ngl
