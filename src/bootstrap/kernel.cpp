#include "kernel.h"
#include <xkelib.h>
#include "../common/ppc.h"

namespace
{
// Bound the search for each compiler epilogue to 512 bytes.
const unsigned MAX_FUNCTION_WORDS = 128;

const unsigned PPC_OPCODE_SHIFT = 26;
const DWORD PPC_OPCODE_BC = 16;     // Conditional branch.
const DWORD PPC_OPCODE_B = 18;      // Direct branch or call.
const DWORD PPC_OPCODE_XL = 19;     // Includes register-indirect branches.
const DWORD PPC_BRANCH_FLAGS = 0x3; // AA (absolute address) and LK (link).
const DWORD PPC_BRANCH_FORM_MASK = 0xFC000000 | PPC_BRANCH_FLAGS;
const DWORD PPC_RELATIVE_B = 0x48000000;
const DWORD PPC_RELATIVE_BL = 0x48000001;
const DWORD PPC_BC_DISPLACEMENT_MASK = 0xFFFC;
const DWORD PPC_D_FORM_MASK = 0xFFFF0000; // Opcode and register operands, excluding the immediate.

// The Xbox 360 compiler's paired __savegprlr_N and __restgprlr_N entries
// are 0x50 bytes apart. The pair identifies the epilogue for this frame.
const DWORD SAVE_TO_RESTORE_OFFSET = 0x50;

struct KernelImage
{
    DWORD base;
    DWORD size;

    bool Read(DWORD address, DWORD &word) const
    {
        if ((address & 3) || address < base || address - base > size - sizeof(DWORD) ||
            !MmIsAddressValid(reinterpret_cast<PVOID>(address)))
            return false;
        word = *reinterpret_cast<const volatile DWORD *>(address);
        return true;
    }
};

DWORD GetKernelExport(HANDLE kernel, DWORD ordinal)
{
    PVOID address = NULL;
    if (!NT_SUCCESS(XexGetProcedureAddress(kernel, ordinal, &address)))
        return 0;
    return reinterpret_cast<DWORD>(address);
}

bool IsRelativeCall(DWORD instruction)
{
    return (instruction & PPC_BRANCH_FORM_MASK) == PPC_RELATIVE_BL;
}

bool HasLocalBranches(const DWORD *words, unsigned wordCount, DWORD address)
{
    // The final branch to __restgprlr_N is the only non-call branch that may leave the body.
    for (unsigned i = 0; i < wordCount - 1; ++i)
    {
        const DWORD instruction = words[i];
        const DWORD pc = address + i * sizeof(DWORD);
        const DWORD opcode = instruction >> PPC_OPCODE_SHIFT;
        if (opcode == PPC_OPCODE_BC)
        {
            const DWORD target = pc + static_cast<SHORT>(instruction & PPC_BC_DISPLACEMENT_MASK);
            if ((instruction & PPC_BRANCH_FLAGS) || target - address >= wordCount * sizeof(DWORD))
                return false;
        }
        else if (opcode == PPC_OPCODE_B && !IsRelativeCall(instruction))
        {
            if ((instruction & PPC_BRANCH_FLAGS) ||
                ppc::RelativeBranchTarget(pc, instruction) - address >= wordCount * sizeof(DWORD))
                return false;
        }
    }
    return true;
}

unsigned ReadFunction(const KernelImage &image, DWORD address, DWORD *words)
{
    for (unsigned i = 0; i < 3; ++i)
        if (!image.Read(address + i * sizeof(DWORD), words[i]))
            return 0;
    // mflr r12; bl __savegprlr_N; stwu r1,-frame(r1)
    if (words[0] != 0x7D8802A6 || !IsRelativeCall(words[1]) || (words[2] & 0xFFFF8000) != 0x94218000)
        return 0;

    const DWORD saveRegisters = ppc::RelativeBranchTarget(address + 4, words[1]);
    const DWORD frameSize = -static_cast<SHORT>(words[2] & 0xFFFF);
    for (unsigned i = 3; i < MAX_FUNCTION_WORDS; ++i)
    {
        const DWORD pc = address + i * sizeof(DWORD);
        // The supported bodies use direct branches. XL-form instructions include bclr and bcctr.
        if (!image.Read(pc, words[i]) || !words[i] || (words[i] >> PPC_OPCODE_SHIFT) == PPC_OPCODE_XL)
            return 0;
        // addi r1,r1,frame; b __restgprlr_N
        if ((words[i] & PPC_BRANCH_FORM_MASK) == PPC_RELATIVE_B && words[i - 1] == (0x38210000 | frameSize) &&
            ppc::RelativeBranchTarget(pc, words[i]) == saveRegisters + SAVE_TO_RESTORE_OFFSET)
            return HasLocalBranches(words, i + 1, address) ? i + 1 : 0;
    }
    return 0;
}

// Both exported loaders finish a successful executable load before releasing the loader lock.
//   bl  XexpFinishExecutableLoad
//   lwz r3,lock(r1)
//   bl  RtlLeaveCriticalSection
DWORD FindFinishCall(const KernelImage &image, DWORD loaderAddress, DWORD leaveCriticalSection)
{
    DWORD words[MAX_FUNCTION_WORDS];
    const unsigned wordCount = ReadFunction(image, loaderAddress, words);
    DWORD finishAddress = 0;
    for (unsigned i = 1; i < wordCount; ++i)
    {
        if (!IsRelativeCall(words[i]) ||
            ppc::RelativeBranchTarget(loaderAddress + i * sizeof(DWORD), words[i]) != leaveCriticalSection)
            continue;
        if (i < 2 || finishAddress || (words[i - 1] & PPC_D_FORM_MASK) != 0x80610000 || !IsRelativeCall(words[i - 2]))
            return 0;
        finishAddress = ppc::RelativeBranchTarget(loaderAddress + (i - 2) * sizeof(DWORD), words[i - 2]);
        if (!finishAddress || finishAddress == leaveCriticalSection)
            return 0;
    }
    return finishAddress;
}

bool VerifyFinishFunction(const KernelImage &image, DWORD address, DWORD executableModuleHandle, DWORD *prologue)
{
    DWORD words[MAX_FUNCTION_WORDS];
    const unsigned wordCount = ReadFunction(image, address, words);
    // ReadFunction checks mflr and bl. The rest of the overwritten prologue is
    // stwu r1,-0x70(r1); mr r30,r3.
    if (wordCount < 8 || words[2] != 0x9421FF90 || words[3] != 0x7C7E1B78)
        return false;

    // The finish routine publishes the module before restoring its stack frame.
    //   lis r11,XexExecutableModuleHandle@ha
    //   stw r30,XexExecutableModuleHandle@l(r11)
    const DWORD loadHandleHigh = words[wordCount - 4];
    const DWORD storeModule = words[wordCount - 3];
    if ((loadHandleHigh & PPC_D_FORM_MASK) != 0x3D600000 || (storeModule & PPC_D_FORM_MASK) != 0x93CB0000 ||
        (loadHandleHigh << 16) + static_cast<SHORT>(storeModule & 0xFFFF) != executableModuleHandle)
        return false;

    // The trampoline calls this helper with r12 holding the caller's original LR.
    static const DWORD saveGprLr29[] = {
        0xFBA1FFE0, // std r29,-32(r1)
        0xFBC1FFE8, // std r30,-24(r1)
        0xFBE1FFF0, // std r31,-16(r1)
        0x9181FFF8, // stw r12,-8(r1)
        0x4E800020, // blr
    };
    const DWORD helper = ppc::RelativeBranchTarget(address + 4, words[1]);
    for (unsigned i = 0; i < ARRAYSIZE(saveGprLr29); ++i)
    {
        DWORD word;
        if (!image.Read(helper + i * sizeof(DWORD), word) || word != saveGprLr29[i])
            return false;
    }
    for (unsigned i = 0; i < 4; ++i)
        prologue[i] = words[i];
    return true;
}
} // namespace

bool xbox::ResolveFinishLoad(KernelHookTarget &target)
{
    HANDLE kernel = NULL;
    if (!NT_SUCCESS(XexGetModuleHandle("xboxkrnl.exe", &kernel)) || !kernel)
        return false;

    const LDR_DATA_TABLE_ENTRY *entry = static_cast<const LDR_DATA_TABLE_ENTRY *>(kernel);
    const KernelImage image = {reinterpret_cast<DWORD>(entry->ImageBase), entry->SizeOfNtImage};
    if (!image.base || image.size < sizeof(DWORD) || image.size > MAXDWORD - image.base)
        return false;

    const DWORD loadExecutable = GetKernelExport(kernel, kernelExp_XexLoadExecutable);
    const DWORD loadImageFromMemory = GetKernelExport(kernel, kernelExp_XexLoadImageFromMemory);
    const DWORD leaveCriticalSection = GetKernelExport(kernel, kernelExp_RtlLeaveCriticalSection);
    const DWORD executableModuleHandle = GetKernelExport(kernel, kernelExp_XexExecutableModuleHandle);
    if (!loadExecutable || !loadImageFromMemory || !leaveCriticalSection || !executableModuleHandle ||
        loadExecutable == loadImageFromMemory)
        return false;

    const DWORD finishAddress = FindFinishCall(image, loadExecutable, leaveCriticalSection);
    if (!finishAddress || finishAddress != FindFinishCall(image, loadImageFromMemory, leaveCriticalSection) ||
        !VerifyFinishFunction(image, finishAddress, executableModuleHandle, target.prologue))
        return false;

    target.address = finishAddress;
    return true;
}
