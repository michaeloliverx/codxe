#include "kernel.h"
#include <xkelib.h>
#include "bootstrap/ppc.h"

namespace
{
const unsigned MAX_FUNCTION_WORDS = 128;

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
    return (instruction & 0xFC000003) == 0x48000001;
}

bool HasLocalBranches(const DWORD *words, unsigned count, DWORD address)
{
    for (unsigned i = 0; i < count - 1; ++i)
    {
        const DWORD instruction = words[i];
        const DWORD pc = address + i * sizeof(DWORD);
        const DWORD opcode = instruction >> 26;
        if (opcode == 16)
        {
            // Conditional branches in these functions stay inside the function.
            const DWORD target = pc + static_cast<SHORT>(instruction & 0xFFFC);
            if ((instruction & 3) || target - address >= count * sizeof(DWORD))
                return false;
        }
        else if (opcode == 18 && !IsRelativeCall(instruction))
        {
            if ((instruction & 3) || codxe::RelativeBranchTarget(pc, instruction) - address >= count * sizeof(DWORD))
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
    if (words[0] != 0x7D8802A6 || !IsRelativeCall(words[1]) || (words[2] & 0xFFFF8000) != 0x94218000)
        return 0; // mflr r12, bl __savegprlr_N, stwu r1,-frame(r1)

    const DWORD saveRegisters = codxe::RelativeBranchTarget(address + 4, words[1]);
    const DWORD frame = 0x10000 - (words[2] & 0xFFFF);
    for (unsigned i = 3; i < MAX_FUNCTION_WORDS; ++i)
    {
        const DWORD pc = address + i * sizeof(DWORD);
        if (!image.Read(pc, words[i]) || !words[i] || words[i] >> 26 == 19)
            return 0;
        // The matched compiler epilogue restores r1 and branches out to __restgprlr_N.
        if ((words[i] & 0xFC000003) == 0x48000000 && words[i - 1] == (0x38210000 | frame) &&
            codxe::RelativeBranchTarget(pc, words[i]) == saveRegisters + 0x50)
            return HasLocalBranches(words, i + 1, address) ? i + 1 : 0;
    }
    return 0;
}

DWORD FindFinishCall(const KernelImage &image, DWORD loader, DWORD leaveCriticalSection)
{
    DWORD words[MAX_FUNCTION_WORDS];
    const unsigned count = ReadFunction(image, loader, words);
    DWORD candidate = 0;
    for (unsigned i = 1; i < count; ++i)
    {
        if (!IsRelativeCall(words[i]) ||
            codxe::RelativeBranchTarget(loader + i * sizeof(DWORD), words[i]) != leaveCriticalSection)
            continue;
        // The finish call is followed by lwz r3,lock(r1) and the call that releases it.
        if (i < 2 || candidate || (words[i - 1] & 0xFFFF0000) != 0x80610000 || !IsRelativeCall(words[i - 2]))
            return 0;
        candidate = codxe::RelativeBranchTarget(loader + (i - 2) * sizeof(DWORD), words[i - 2]);
        if (!candidate || candidate == leaveCriticalSection)
            return 0;
    }
    return candidate;
}

bool VerifyFinishFunction(const KernelImage &image, DWORD address, DWORD executableModuleHandle, DWORD *prologue)
{
    DWORD words[MAX_FUNCTION_WORDS];
    const unsigned count = ReadFunction(image, address, words);
    if (count < 8 || words[2] != 0x9421FF90 || words[3] != 0x7C7E1B78)
        return false;

    // The epilogue publishes the executable module through its exported global.
    const DWORD high = words[count - 4];
    const DWORD store = words[count - 3];
    if ((high & 0xFFFF0000) != 0x3D600000 || (store & 0xFFFF0000) != 0x93CB0000 ||
        (high << 16) + static_cast<SHORT>(store & 0xFFFF) != executableModuleHandle)
        return false; // lis r11,handle@ha, stw r30,handle@l(r11)

    // The trampoline calls this helper out of line, so verify its register saves and return.
    static const DWORD saveGprLr29[] = {0xFBA1FFE0, 0xFBC1FFE8, 0xFBE1FFF0, 0x9181FFF8, 0x4E800020};
    const DWORD helper = codxe::RelativeBranchTarget(address + 4, words[1]);
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

    const DWORD disk = GetKernelExport(kernel, kernelExp_XexLoadExecutable);
    const DWORD memory = GetKernelExport(kernel, kernelExp_XexLoadImageFromMemory);
    const DWORD leave = GetKernelExport(kernel, kernelExp_RtlLeaveCriticalSection);
    const DWORD executable = GetKernelExport(kernel, kernelExp_XexExecutableModuleHandle);
    if (!disk || !memory || !leave || !executable || disk == memory)
        return false;

    const DWORD address = FindFinishCall(image, disk, leave);
    if (!address || address != FindFinishCall(image, memory, leave) ||
        !VerifyFinishFunction(image, address, executable, target.prologue))
        return false;

    target.address = address;
    return true;
}
