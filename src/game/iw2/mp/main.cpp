#include "components/scr_parser.h"
#include "main.h"
#include "common.h"

namespace iw2
{
namespace mp
{

IW2_MP_Plugin::IW2_MP_Plugin()
{
    DbgPrint("IW2 MP Plugin initialized\n");
    RegisterModule(new scr_parser());
}

IW2_MP_Plugin::~IW2_MP_Plugin()
{
}

bool IW2_MP_Plugin::ShouldLoad()
{
    return (strncmp((char *)0x820410E4, "multiplayer", 11) == 0);
}
} // namespace mp
} // namespace iw2
