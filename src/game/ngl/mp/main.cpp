#include "pch.h"
#include "main.h"

namespace ngl
{
namespace mp
{

#include <stdint.h>

#define TARGET_ADDR 0x8244C1C8
#define TOTAL_SIZE 388


void ApplyResearchPatches()
{
    // --- 1. Single Instruction Write ---
    // Set NOP (0x60000000) at 0x82466704
    *(volatile uint32_t *)0x82466704 = 0x60000000;

    // Destination memory address
    volatile uint32_t *dst = (volatile uint32_t *)0x8244C1C8;

    // Full 55-word shellcode
    const uint32_t shellcode[] = {0x3D603A83, 0x616B126F, 0x9161FFFC, 0x39600000, 0x9161FFF8, 0x3D603F80, 0x9161FFF4,
                                  0xC0030004, 0x60000000, 0xEC000032, 0xC1440008, 0xFD605210, 0xC1830000, 0xEDAC033A,
                                  0xC001FFFC, 0xFF0B0000, 0x41980084, 0x60000000, 0xC121FFF8, 0xFF0D4800, 0x419A0074,
                                  0xC0040000, 0x60000000, 0xED000332, 0xC0E40004, 0xC1630004, 0xC0030008, 0xED0742FA,
                                  0xECE0683A, 0xEC085024, 0xC141FFF4, 0xFC000050, 0xEDA0683A, 0xEDA76824, 0xEDA0682C,
                                  0xFF0D5000, 0x41980018, 0xFF004800, 0x41980010, 0xC1430008, 0xFF0A4800, 0x4C990020,
                                  0xED8C0372, 0xD1850000, 0xED8B0372, 0xD1850004, 0xEC0D0032, 0xD0050008, 0x4E800020,
                                  0xD1850000, 0xC0030004, 0xD0050004, 0xC0030008, 0xD0050008, 0x4E800020};

    // shellcodeSize = 55
    int shellcodeSize = sizeof(shellcode) / sizeof(uint32_t);

    // 1. Writes exactly 55 words (220 bytes)
    // Ends with 0x4E800020
    for (int i = 0; i < shellcodeSize; i++)
    {
        dst[i] = shellcode[i];
    }

    // 2. Writes exactly 59 NOPs (236 bytes)
    // Starts immediately after the last shellcode word
    for (int i = 0; i < 59; i++)
    {
        dst[shellcodeSize + i] = 0x60000000;
    }

    // 3. Branch patch at specific address
    *(volatile uint32_t *)0x8246E1E0 = 0x4BFDDFE9;
}


NGL_MP_Plugin::NGL_MP_Plugin()
{
    DbgPrint("NGL MP Plugin initialized\n");
    ApplyResearchPatches();
    RegisterModule(new Config());
}

NGL_MP_Plugin::~NGL_MP_Plugin()
{
    DbgPrint("NGL MP Plugin shutting down\n");
}

} // namespace mp
} // namespace ngl
