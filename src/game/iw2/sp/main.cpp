#include "components/scr_parser.h"
#include "main.h"
#include "common.h"

namespace iw2
{
namespace sp
{

IW2_SP_Plugin::IW2_SP_Plugin()
{
    DbgPrint("IW2 SP Plugin initialized\n");
    this->RegisterModule(new scr_parser());
}

IW2_SP_Plugin::~IW2_SP_Plugin()
{
}

bool IW2_SP_Plugin::ShouldLoad()
{
    // Check if the game is in single player mode
    return (strncmp((char *)0x82059FE0, "startSingleplayer", 17) == 0);
}

} // namespace sp
} // namespace iw2
