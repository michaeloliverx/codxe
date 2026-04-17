#include "pch.h"
#include "components/gsc.h"
#include "components/scr_parser.h"
#include "components/project_velocity.h"
#include "main.h"

namespace iw2
{
namespace mp
{

IW2_MP_Plugin::IW2_MP_Plugin()
{
    RegisterModule(new Config());
    RegisterModule(new GSC());
    RegisterModule(new scr_parser());
    RegisterModule(new project_velocity());
}

IW2_MP_Plugin::~IW2_MP_Plugin()
{
}

} // namespace mp
} // namespace iw2
