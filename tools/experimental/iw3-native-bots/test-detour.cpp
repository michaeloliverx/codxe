// Exercise the actual detour emitter, then interpret the relevant PowerPC instructions.
#include "../../../src/detour.h"
#include <cstdlib>
#include <iostream>
#include <map>

BYTE Detour::TrampolineBuffer[200 * 20] = {};
SIZE_T Detour::TrampolineSize = 0;

#define CHECK(condition)                                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(condition))                                                                                              \
        {                                                                                                              \
            std::cerr << "FAIL line " << __LINE__ << ": " << #condition << '\n';                                       \
            std::exit(1);                                                                                              \
        }                                                                                                              \
    } while (0)

struct Cpu
{
    UINT64 reg[32];
    UINT32 lr, ctr;
    std::map<UINT64, UINT64> stack;
    Cpu() : lr(0x12345678), ctr(0)
    {
        for (int i = 0; i < 32; ++i)
            reg[i] = 0x1000000000000000ULL + i;
        reg[0] = 0xBEEF0000;
        reg[1] = 0x30000000;
    }

    void Execute(UINT32 instruction)
    {
        if (instruction == 0x7D8802A6) // mflr r12
        {
            reg[12] = lr;
            return;
        }
        if ((instruction & ~0x03E00000u) == 0x7C0903A6) // mtctr rX
        {
            ctr = static_cast<UINT32>(reg[(instruction >> 21) & 31]);
            return;
        }
        const unsigned op = instruction >> 26;
        const unsigned rt = (instruction >> 21) & 31, ra = (instruction >> 16) & 31;
        const INT16 displacement = static_cast<INT16>(instruction & 0xFFFC);
        switch (op)
        {
        case 15: // lis
            CHECK(ra == 0);
            reg[rt] = static_cast<UINT64>(static_cast<INT64>(static_cast<INT16>(instruction)) * 65536);
            break;
        case 24: // ori
            reg[ra] = reg[rt] | (instruction & 0xFFFF);
            break;
        case 62: // std
            stack[reg[ra] + displacement] = reg[rt];
            break;
        case 58: // ld
            CHECK(stack.count(reg[ra] + displacement));
            reg[rt] = stack[reg[ra] + displacement];
            break;
        case 54: // stfd: only its address matters to this saved-register test
            stack[reg[ra] + static_cast<INT16>(instruction)] = 0xF0000000ULL + rt;
            break;
        case 37: // stwu
        {
            const UINT64 destination = reg[ra] + static_cast<INT16>(instruction);
            stack[destination] = reg[rt];
            reg[ra] = destination;
            break;
        }
        default:
            CHECK(false);
        }
    }
};

void Run(SIZE_T minimumLength, bool preservesR27)
{
    // Exact TU4 G_RunFrame prologue at 0x822731F8. The fifth instruction allocates the frame.
    const UINT32 prologue[] = {0x7D8802A6, 0x4BEAE0D9, 0xDBC1FF80, 0xDBE1FF88, 0x9421F6D0};
    auto *source = static_cast<UINT32 *>(
        VirtualAlloc(reinterpret_cast<void *>(0x10000000), 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    CHECK(source != nullptr);
    memcpy(source, prologue, sizeof(prologue));
    Detour::ResetTrampolinePool();
    Detour detour(source, reinterpret_cast<void *>(0x20000000));
    CHECK(!detour.Install(18) && !detour.Install(32));
    CHECK(memcmp(source, prologue, sizeof(prologue)) == 0);
    CHECK(detour.Install(minimumLength));
    const SIZE_T copied = minimumLength > 16 ? minimumLength : 16;
    const UINT32 *program = detour.GetOriginal<const UINT32 *>();
    const UINT32 begin = static_cast<UINT32>(reinterpret_cast<UINT_PTR>(program));
    const UINT32 exit = static_cast<UINT32>(reinterpret_cast<UINT_PTR>(source)) + static_cast<UINT32>(copied);
    INT32 branchOffset = prologue[1] & 0x03FFFFFC;
    if (branchOffset & 0x02000000)
        branchOffset |= static_cast<INT32>(0xFC000000);
    const UINT32 helper = static_cast<UINT32>(reinterpret_cast<UINT_PTR>(source)) + 4 + branchOffset;
    Cpu cpu;
    const UINT64 oldStack = cpu.reg[1];
    const UINT64 originalR27 = cpu.reg[27];
    UINT32 pc = begin;
    bool finished = false;
    for (unsigned step = 0; step < 100 && !finished; ++step)
    {
        CHECK(pc >= begin && pc - begin < Detour::GetTrampolinePoolSize());
        const UINT32 instruction = program[(pc - begin) / 4];
        if ((instruction & ~1u) == 0x4E800420) // bctr / bctrl
        {
            if (instruction & 1)
                cpu.lr = pc + 4;
            if (cpu.ctr == helper)
            {
                CHECK(instruction & 1);
                // __savegprlr_19: r27 is saved at -0x30 from the pre-allocation stack pointer.
                for (int r = 19; r <= 31; ++r)
                    cpu.stack[cpu.reg[1] - 0x70 + (r - 19) * 8] = cpu.reg[r];
                cpu.stack[cpu.reg[1] - 8] = cpu.reg[12];
                pc = cpu.lr;
            }
            else
            {
                CHECK(cpu.ctr == exit && !(instruction & 1));
                finished = true;
            }
        }
        else
        {
            cpu.Execute(instruction);
            pc += 4;
        }
    }
    CHECK(finished);
    for (SIZE_T i = copied / 4; i < sizeof(prologue) / sizeof(prologue[0]); ++i)
        cpu.Execute(prologue[i]);
    CHECK(cpu.reg[1] == oldStack - 0x930);
    // Complete the corresponding epilogue; focus on the saved-register state, not the game body.
    cpu.reg[1] += 0x930;
    for (int r = 19; r <= 31; ++r)
        cpu.reg[r] = cpu.stack[cpu.reg[1] - 0x70 + (r - 19) * 8];
    CHECK((cpu.reg[27] == originalR27) == preservesR27);
    if (!preservesR27)
        CHECK(cpu.reg[27] == 0xBEEF0000);
    for (int r = 19; r <= 31; ++r)
        if (r != 27)
            CHECK(cpu.reg[r] == 0x1000000000000000ULL + r);
    CHECK(detour.Remove());
    CHECK(memcmp(source, prologue, sizeof(prologue)) == 0);
    CHECK(VirtualFree(source, 0, MEM_RELEASE));
}

int main()
{
    Run(0, false); // Reproduce the observed 16-byte prologue corruption with the existing default.
    Run(20, true); // Copying through stwu preserves every nonvolatile register tested.
    std::cout
        << "PASS: actual detour emitter reproduces r27 corruption at 16 bytes; 20-byte TU4 prologue preserves it; "
           "length validation and unhook restoration pass\n";
}
