#include "pch.h"
#include "components/scr_parser.h"
#include "main.h"

namespace iw2
{
namespace mp
{

IW2_MP_Plugin::IW2_MP_Plugin()
{
    DbgPrint("IW2 MP Plugin initialized\n");
    RegisterModule(new Config());
    RegisterModule(new scr_parser());
}

IW2_MP_Plugin::~IW2_MP_Plugin()
{
}

} // namespace mp
} // namespace iw2
