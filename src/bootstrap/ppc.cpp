#include "ppc.h"

void codxe::WriteBranch(volatile DWORD *out, DWORD target, bool linked)
{
    out[0] = 0x3D600000 | (target >> 16);    // lis r11
    out[1] = 0x616B0000 | (target & 0xffff); // ori r11,r11
    out[2] = 0x7D6903A6;                     // mtctr r11
    out[3] = 0x4E800420 | (linked ? 1 : 0);  // bctr/bctrl
}

DWORD codxe::RelativeBranchTarget(DWORD address, DWORD instruction)
{
    DWORD displacement = instruction & 0x03FFFFFC;
    if (displacement & 0x02000000)
        displacement |= 0xFC000000;
    return address + displacement;
}

void codxe::BuildLoaderTrampoline(const DWORD source[4], DWORD address, DWORD out[11])
{
    // The resolver verifies mflr, bl __savegprlr_29, stwu, mr.
    out[0] = source[0];
    WriteBranch(out + 1, RelativeBranchTarget(address + 4, source[1]), true);
    out[5] = source[2];
    out[6] = source[3];
    WriteBranch(out + 7, address + 16, false);
}
