#include "pch.h"
#include "components/g_scr_main.h"
#include "components/scr_parser.h"
#include "main.h"

namespace iw2
{
namespace mp
{

IW2_MP_Plugin::IW2_MP_Plugin()
{
    RegisterModule(new Config());
    RegisterModule(new g_scr_main());
    RegisterModule(new scr_parser());
}

IW2_MP_Plugin::~IW2_MP_Plugin()
{
}

} // namespace mp
} // namespace iw2
