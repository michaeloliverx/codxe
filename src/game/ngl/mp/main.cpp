#include "pch.h"
#include "main.h"
#include "components/project_velocity.h"

namespace ngl
{
namespace mp
{

NGL_MP_Plugin::NGL_MP_Plugin()
{
    DbgPrint("NGL MP Plugin initialized\n");

    // install_patch();
    RegisterModule(new project_velocity());
}

NGL_MP_Plugin::~NGL_MP_Plugin()
{
    DbgPrint("NGL MP Plugin shutting down\n");
}

} // namespace mp
} // namespace ngl