#include "pch.h"
#include "patches.h"

namespace iw3
{
namespace mp
{
namespace
{
void ExpandPMem()
{
    // Xenia has its own TOML patch that has a higher limit
    // Maybe we should do it at runtime here as well :shrug:
    if (xbox::GetEnvironment() == xbox::ENVIRONMENT_XENIA)
        return;

    // Increase memory allocation by 10 MiB which gives room for larger fastfiles.
    // PMem_Init: 0x19700000 + 10 MiB = 0x1A100000.
    // Keep the allocation request and the retry-size reference in sync.
    *(volatile uint16_t *)0x821CFD1A = 0x1A10;
    *(volatile uint16_t *)0x821CFD2A = 0x1A10;
}
} // namespace

Patches::Patches()
{
    ExpandPMem();
}
} // namespace mp
} // namespace iw3
