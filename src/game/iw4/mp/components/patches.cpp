#include "pch.h"
#include "patches.h"

namespace iw4
{
namespace mp
{
void DisableFastfileAuth()
{
    // fastfile_allowNoAuth
    // Allow loading fastfiles with no auth.
    // .text:8216A18C                 beq       cr6, loc_8216A1B0
    *(volatile uint32_t *)0x8216A18C = 0x60000000;
}

void EnableBouncing()
{
    // .text:820DABE0                 cmpwi     cr6, r23, 0
    // .text:820DABE4                 bne       cr6, loc_820DAB94
    *(volatile uint32_t *)0x820DABE0 = 0x60000000;
    *(volatile uint32_t *)0x820DABE4 = 0x60000000;
}

patches::patches()
{
    DisableFastfileAuth();
    EnableBouncing();
}
} // namespace mp
} // namespace iw4
