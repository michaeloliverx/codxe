#include "hooks/detour.h"

#include <cstring>
#include <cstdint>

#define POWERPC_MASK_N_BITS(bits) ((1 << (bits)) - 1)
#define POWERPC_HI(value) (((value) >> 16) & 0xFFFF)
#define POWERPC_LO(value) ((value) & 0xFFFF)
#define POWERPC_BIT32(bit) (31 - (bit))
#define POWERPC_OPCODE(opcode) ((opcode) << 26)
#define POWERPC_EXTENDED_OPCODE(opcode) ((opcode) << 1)
#define POWERPC_SPR(spr) (static_cast<UINT32>((((spr) & 0x1F) << 5) | (((spr) >> 5) & 0x1F)))

#define POWERPC_OPCODE_ADDI POWERPC_OPCODE(14)
#define POWERPC_OPCODE_ADDIS POWERPC_OPCODE(15)
#define POWERPC_OPCODE_BC POWERPC_OPCODE(16)
#define POWERPC_OPCODE_B POWERPC_OPCODE(18)
#define POWERPC_OPCODE_BCCTR POWERPC_OPCODE(19)
#define POWERPC_OPCODE_ORI POWERPC_OPCODE(24)
#define POWERPC_OPCODE_EXTENDED POWERPC_OPCODE(31)
#define POWERPC_OPCODE_MASK POWERPC_OPCODE(63)
#define POWERPC_OPCODE_LD POWERPC_OPCODE(58)
#define POWERPC_OPCODE_STD POWERPC_OPCODE(62)

#define POWERPC_EXTENDED_OPCODE_BCCTR POWERPC_EXTENDED_OPCODE(528)
#define POWERPC_EXTENDED_OPCODE_MTSPR POWERPC_EXTENDED_OPCODE(467)

#define POWERPC_ADDI(rD, rA, SIMM)                                                                                     \
    static_cast<UINT32>(POWERPC_OPCODE_ADDI | ((rD) << POWERPC_BIT32(10)) | ((rA) << POWERPC_BIT32(15)) | (SIMM))
#define POWERPC_ADDIS(rD, rA, SIMM)                                                                                    \
    static_cast<UINT32>(POWERPC_OPCODE_ADDIS | ((rD) << POWERPC_BIT32(10)) | ((rA) << POWERPC_BIT32(15)) | (SIMM))
#define POWERPC_LIS(rD, SIMM) POWERPC_ADDIS((rD), 0, (SIMM))
#define POWERPC_MTSPR(spr, rS)                                                                                         \
    static_cast<UINT32>(POWERPC_OPCODE_EXTENDED | ((rS) << POWERPC_BIT32(10)) |                                        \
                        (POWERPC_SPR(spr) << POWERPC_BIT32(20)) | POWERPC_EXTENDED_OPCODE_MTSPR)
#define POWERPC_MTCTR(rS) POWERPC_MTSPR(9, (rS))
#define POWERPC_ORI(rS, rA, UIMM)                                                                                      \
    static_cast<UINT32>(POWERPC_OPCODE_ORI | ((rS) << POWERPC_BIT32(10)) | ((rA) << POWERPC_BIT32(15)) | (UIMM))
#define POWERPC_BCCTR(BO, BI, LK)                                                                                      \
    static_cast<UINT32>(POWERPC_OPCODE_BCCTR | ((BO) << POWERPC_BIT32(10)) | ((BI) << POWERPC_BIT32(15)) |             \
                        ((LK) & 1) | POWERPC_EXTENDED_OPCODE_BCCTR)
#define POWERPC_STD(rS, DS, rA)                                                                                        \
    static_cast<UINT32>(POWERPC_OPCODE_STD | ((rS) << POWERPC_BIT32(10)) | ((rA) << POWERPC_BIT32(15)) |               \
                        (static_cast<INT16>(DS) & 0xFFFF))
#define POWERPC_LD(rS, DS, rA)                                                                                         \
    static_cast<UINT32>(POWERPC_OPCODE_LD | ((rS) << POWERPC_BIT32(10)) | ((rA) << POWERPC_BIT32(15)) |                \
                        (static_cast<INT16>(DS) & 0xFFFF))

namespace codxe
{
namespace
{

const UINT32 kPowerpcBranchLinked = 1;
const UINT32 kPowerpcBranchAbsolute = 2;
const UINT32 kPowerpcBranchOptionsAlways = 20;
const SIZE_T kTrampolineBufferSize = 200 * 20;

extern "C"
{
    NTSYSAPI BOOL NTAPI MmIsAddressValid(IN PVOID address);
}

#pragma section(".text")
__declspec(allocate(".text")) BYTE s_trampoline_buffer[kTrampolineBufferSize] = {};
SIZE_T s_trampoline_size = 0;

SIZE_T WriteFarBranchEx(void *destination, const void *branch_target, bool linked, bool preserve_register,
                        UINT32 branch_options, BYTE condition_register_bit, BYTE register_index)
{
    const UINT32 branch_far[] = {
        POWERPC_LIS(register_index, POWERPC_HI(reinterpret_cast<UINT32>(branch_target))),
        POWERPC_ORI(register_index, register_index, POWERPC_LO(reinterpret_cast<UINT32>(branch_target))),
        POWERPC_MTCTR(register_index),
        POWERPC_BCCTR(branch_options, condition_register_bit, linked),
    };

    const UINT32 branch_far_preserve[] = {
        POWERPC_STD(register_index, -0x30, 1),
        POWERPC_LIS(register_index, POWERPC_HI(reinterpret_cast<UINT32>(branch_target))),
        POWERPC_ORI(register_index, register_index, POWERPC_LO(reinterpret_cast<UINT32>(branch_target))),
        POWERPC_MTCTR(register_index),
        POWERPC_LD(register_index, -0x30, 1),
        POWERPC_BCCTR(branch_options, condition_register_bit, linked),
    };

    const void *branch_code = preserve_register ? branch_far_preserve : branch_far;
    const SIZE_T branch_size = preserve_register ? sizeof(branch_far_preserve) : sizeof(branch_far);

    if (destination != 0)
        memcpy(destination, branch_code, branch_size);

    return branch_size;
}

SIZE_T WriteFarBranch(void *destination, const void *branch_target, bool linked, bool preserve_register)
{
    return WriteFarBranchEx(destination, branch_target, linked, preserve_register, kPowerpcBranchOptionsAlways, 0, 0);
}

bool ReserveTrampolineBytes(SIZE_T bytes)
{
    return s_trampoline_size + bytes <= kTrampolineBufferSize;
}

SIZE_T RelocateBranch(UINT32 *destination, const UINT32 *source)
{
    const UINT32 instruction = *source;
    const UINT32 instruction_address = reinterpret_cast<UINT32>(source);

    if ((instruction & kPowerpcBranchAbsolute) != 0)
    {
        *destination = instruction;
        return sizeof(UINT32);
    }

    INT32 branch_offset_bit_size = 0;
    INT32 branch_offset_bit_base = 0;
    UINT32 branch_options = kPowerpcBranchOptionsAlways;
    BYTE condition_register_bit = 0;

    switch (instruction & POWERPC_OPCODE_MASK)
    {
    case POWERPC_OPCODE_B:
        branch_offset_bit_size = 24;
        branch_offset_bit_base = 2;
        break;
    case POWERPC_OPCODE_BC:
        branch_offset_bit_size = 14;
        branch_offset_bit_base = 2;
        branch_options = (instruction >> POWERPC_BIT32(10)) & POWERPC_MASK_N_BITS(5);
        condition_register_bit = static_cast<BYTE>((instruction >> POWERPC_BIT32(15)) & POWERPC_MASK_N_BITS(5));
        break;
    default:
        *destination = instruction;
        return sizeof(UINT32);
    }

    INT32 branch_offset = instruction & (POWERPC_MASK_N_BITS(branch_offset_bit_size) << branch_offset_bit_base);
    if (branch_offset >> ((branch_offset_bit_size + branch_offset_bit_base) - 1))
        branch_offset |= ~POWERPC_MASK_N_BITS(branch_offset_bit_size + branch_offset_bit_base);

    const void *branch_address = reinterpret_cast<const void *>(instruction_address + branch_offset);
    return WriteFarBranchEx(destination, branch_address, (instruction & kPowerpcBranchLinked) != 0, true,
                            branch_options, condition_register_bit, 0);
}

SIZE_T CopyInstruction(UINT32 *destination, const UINT32 *source)
{
    const UINT32 instruction = *source;

    switch (instruction & POWERPC_OPCODE_MASK)
    {
    case POWERPC_OPCODE_B:
    case POWERPC_OPCODE_BC:
        return RelocateBranch(destination, source);
    default:
        *destination = instruction;
        return sizeof(UINT32);
    }
}

} // namespace

void DetourInit(Detour *detour, void *source, const void *target)
{
    if (detour == 0)
        return;

    detour->source = source;
    detour->target = target;
    detour->trampoline = 0;
    detour->original_length = 0;
    memset(detour->original_instructions, 0, sizeof(detour->original_instructions));
}

bool DetourInstall(Detour *detour)
{
    if (detour == 0 || detour->source == 0 || detour->target == 0 || detour->original_length != 0)
        return false;

    const SIZE_T hook_size = WriteFarBranch(0, detour->target, false, false);
    if (hook_size > sizeof(detour->original_instructions))
        return false;

    const SIZE_T trampoline_return_size = WriteFarBranch(0, detour->source, false, true);
    if (!ReserveTrampolineBytes(hook_size * 2 + trampoline_return_size))
        return false;

    memcpy(detour->original_instructions, detour->source, hook_size);
    detour->original_length = hook_size;
    detour->trampoline = &s_trampoline_buffer[s_trampoline_size];

    for (SIZE_T i = 0; i < hook_size / sizeof(UINT32); ++i)
    {
        const UINT32 *instruction =
            reinterpret_cast<const UINT32 *>(reinterpret_cast<UINT32>(detour->source) + (i * sizeof(UINT32)));
        s_trampoline_size +=
            CopyInstruction(reinterpret_cast<UINT32 *>(&s_trampoline_buffer[s_trampoline_size]), instruction);
    }

    const void *after_branch = reinterpret_cast<const void *>(reinterpret_cast<UINT32>(detour->source) + hook_size);
    s_trampoline_size += WriteFarBranch(&s_trampoline_buffer[s_trampoline_size], after_branch, false, true);

    WriteFarBranch(detour->source, detour->target, false, false);
    return true;
}

bool DetourRemove(Detour *detour)
{
    if (detour == 0 || detour->source == 0 || detour->original_length == 0)
        return false;

    if (MmIsAddressValid(detour->source))
        memcpy(detour->source, detour->original_instructions, detour->original_length);

    detour->source = 0;
    detour->target = 0;
    detour->trampoline = 0;
    detour->original_length = 0;
    memset(detour->original_instructions, 0, sizeof(detour->original_instructions));
    return true;
}

SIZE_T DetourGetTrampolinePoolSize()
{
    return s_trampoline_size;
}

void DetourResetTrampolinePool(SIZE_T size)
{
    if (size <= kTrampolineBufferSize)
        s_trampoline_size = size;
}

} // namespace codxe
