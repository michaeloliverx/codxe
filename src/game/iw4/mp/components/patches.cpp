#include "pch.h"
#include "patches.h"

namespace iw4
{
namespace mp
{
void EnableBouncing()
{
    // .text:820DABE0                 cmpwi     cr6, r23, 0
    // .text:820DABE4                 bne       cr6, loc_820DAB94
    ppc::Nop(0x820DABE0);
    ppc::Nop(0x820DABE4);
}

patches::patches()
{
    EnableBouncing();
}
} // namespace mp
} // namespace iw4
