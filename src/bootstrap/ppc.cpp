#include "ppc.h"

void codxe::WriteBranch(volatile DWORD *out, DWORD target, bool linked)
{
    out[0] = 0x3D600000 | (target >> 16);    // lis r11
    out[1] = 0x616B0000 | (target & 0xffff); // ori r11,r11
    out[2] = 0x7D6903A6;                     // mtctr r11
    out[3] = 0x4E800420 | (linked ? 1 : 0);  // bctr/bctrl
}

void codxe::BuildLoaderTrampoline(const DWORD source[4], DWORD address, DWORD out[11])
{
    // Every verified profile starts with mflr, bl __savegprlr_N, stwu, mr.
    DWORD displacement = source[1] & 0x03FFFFFC;
    if (displacement & 0x02000000)
        displacement |= 0xFC000000;
    out[0] = source[0];
    WriteBranch(out + 1, address + 4 + displacement, true);
    out[5] = source[2];
    out[6] = source[3];
    WriteBranch(out + 7, address + 16, false);
}
