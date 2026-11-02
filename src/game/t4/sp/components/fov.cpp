#include "pch.h"
#include "fov.h"
#include "fov_math.h"

extern "C" VOID NTAPI KeFlushCacheRange(PVOID address, DWORD size);

namespace t4
{
namespace sp
{
namespace
{
const dvar_s *userFov = nullptr;
const dvar_s *userScale = nullptr;
bool hooksInstalled = false;

float ApplyUserFov(float stock)
{
    const auto *minimum = *reinterpret_cast<dvar_s **>(0x844103B0);
    return userFov && minimum ? fov::ApplyBase(stock, userFov->current.value, minimum->current.value) : stock;
}

float ApplyUserScale(float stock)
{
    const auto *scale = *reinterpret_cast<dvar_s **>(0x8459BA20);
    return userScale && scale ? fov::ApplyScale(stock, userScale->current.value, scale->domain.value.min,
                                               scale->domain.value.max) : stock;
}

// Mid-function hooks must preserve volatile state across the C++ helper call.
// The checked TU7 sites have dead r0/CTR; far branches use only those registers.
__declspec(naked) void FovThunk()
{
    __asm
    {
        stwu r1, -0x180(r1)
        std r3, 0x50(r1)
        std r4, 0x58(r1)
        std r5, 0x60(r1)
        std r6, 0x68(r1)
        std r7, 0x70(r1)
        std r8, 0x78(r1)
        std r9, 0x80(r1)
        std r10, 0x88(r1)
        std r11, 0x90(r1)
        std r12, 0x98(r1)
        stfd fr0, 0xA0(r1)
        stfd fr1, 0xA8(r1)
        stfd fr2, 0xB0(r1)
        stfd fr3, 0xB8(r1)
        stfd fr4, 0xC0(r1)
        stfd fr5, 0xC8(r1)
        stfd fr6, 0xD0(r1)
        stfd fr7, 0xD8(r1)
        stfd fr8, 0xE0(r1)
        stfd fr9, 0xE8(r1)
        stfd fr10, 0xF0(r1)
        stfd fr11, 0xF8(r1)
        stfd fr12, 0x100(r1)
        stfd fr13, 0x108(r1)
        mflr r0
        std r0, 0x110(r1)
        mfcr r0
        stw r0, 0x118(r1)
        mfxer r0
        std r0, 0x128(r1)
        mffs fr0
        stfd fr0, 0x130(r1)
        bl ApplyUserFov
        stfs fr1, 0x120(r1)
        lfd fr0, 0x130(r1)
        mtfsf 255, fr0
        ld r0, 0x128(r1)
        mtxer r0
        lwz r0, 0x118(r1)
        mtcr r0
        ld r0, 0x110(r1)
        mtlr r0
        ld r3, 0x50(r1)
        ld r4, 0x58(r1)
        ld r5, 0x60(r1)
        ld r6, 0x68(r1)
        ld r7, 0x70(r1)
        ld r8, 0x78(r1)
        ld r9, 0x80(r1)
        ld r10, 0x88(r1)
        ld r11, 0x90(r1)
        ld r12, 0x98(r1)
        lfd fr0, 0xA0(r1)
        lfd fr1, 0xA8(r1)
        lfd fr2, 0xB0(r1)
        lfd fr3, 0xB8(r1)
        lfd fr4, 0xC0(r1)
        lfd fr5, 0xC8(r1)
        lfd fr6, 0xD0(r1)
        lfd fr7, 0xD8(r1)
        lfd fr8, 0xE0(r1)
        lfd fr9, 0xE8(r1)
        lfd fr10, 0xF0(r1)
        lfd fr11, 0xF8(r1)
        lfd fr12, 0x100(r1)
        lfd fr13, 0x108(r1)
        lfs fr1, 0x120(r1)
        addi r1, r1, 0x180
        blr
    }
}

__declspec(naked) void ScaleThunk()
{
    __asm
    {
        stwu r1, -0x180(r1)
        std r3, 0x50(r1)
        std r4, 0x58(r1)
        std r5, 0x60(r1)
        std r6, 0x68(r1)
        std r7, 0x70(r1)
        std r8, 0x78(r1)
        std r9, 0x80(r1)
        std r10, 0x88(r1)
        std r11, 0x90(r1)
        std r12, 0x98(r1)
        stfd fr0, 0xA0(r1)
        stfd fr1, 0xA8(r1)
        stfd fr2, 0xB0(r1)
        stfd fr3, 0xB8(r1)
        stfd fr4, 0xC0(r1)
        stfd fr5, 0xC8(r1)
        stfd fr6, 0xD0(r1)
        stfd fr7, 0xD8(r1)
        stfd fr8, 0xE0(r1)
        stfd fr9, 0xE8(r1)
        stfd fr10, 0xF0(r1)
        stfd fr11, 0xF8(r1)
        stfd fr12, 0x100(r1)
        stfd fr13, 0x108(r1)
        mflr r0
        std r0, 0x110(r1)
        mfcr r0
        stw r0, 0x118(r1)
        mfxer r0
        std r0, 0x128(r1)
        mffs fr0
        stfd fr0, 0x130(r1)
        bl ApplyUserScale
        stfs fr1, 0x120(r1)
        lfd fr0, 0x130(r1)
        mtfsf 255, fr0
        ld r0, 0x128(r1)
        mtxer r0
        lwz r0, 0x118(r1)
        mtcr r0
        ld r0, 0x110(r1)
        mtlr r0
        ld r3, 0x50(r1)
        ld r4, 0x58(r1)
        ld r5, 0x60(r1)
        ld r6, 0x68(r1)
        ld r7, 0x70(r1)
        ld r8, 0x78(r1)
        ld r9, 0x80(r1)
        ld r10, 0x88(r1)
        ld r11, 0x90(r1)
        ld r12, 0x98(r1)
        lfd fr0, 0xA0(r1)
        lfd fr1, 0xA8(r1)
        lfd fr2, 0xB0(r1)
        lfd fr3, 0xB8(r1)
        lfd fr4, 0xC0(r1)
        lfd fr5, 0xC8(r1)
        lfd fr6, 0xD0(r1)
        lfd fr7, 0xD8(r1)
        lfd fr8, 0xE0(r1)
        lfd fr9, 0xE8(r1)
        lfd fr10, 0xF0(r1)
        lfd fr11, 0xF8(r1)
        lfd fr12, 0x100(r1)
        lfd fr13, 0x108(r1)
        lfs fr1, 0x120(r1)
        addi r1, r1, 0x180
        blr
    }
}

__declspec(naked) void BaseFovHook()
{
    __asm
    {
        addi r28, r11, -4008
        lwz r11, -11348(r27)
        lwzx r29, r8, r28
        lfs fr28, 16(r11)
        mflr r0
        stw r0, -8(r1)
        stwu r1, -0x60(r1)
        stfd fr1, 0x50(r1)
        fmr fr1, fr28
        bl FovThunk
        fmr fr28, fr1
        lfd fr1, 0x50(r1)
        addi r1, r1, 0x60
        lwz r0, -8(r1)
        mtlr r0
        lis r0, 0x8210
        ori r0, r0, 0x7C28
        mtctr r0
        bctr
    }
}

__declspec(naked) void RecoveryFovHook()
{
    __asm
    {
        lfd fr13, 0x50(r1)
        fcfid fr11, fr0
        fcfid fr12, fr13
        lfs fr10, 16(r11)
        mflr r0
        stw r0, -8(r1)
        stwu r1, -0x60(r1)
        stfd fr1, 0x50(r1)
        fmr fr1, fr10
        bl FovThunk
        fmr fr10, fr1
        lfd fr1, 0x50(r1)
        addi r1, r1, 0x60
        lwz r0, -8(r1)
        mtlr r0
        lis r0, 0x8210
        ori r0, r0, 0x7D10
        mtctr r0
        bctr
    }
}

__declspec(naked) void ScaleFovHook()
{
    __asm
    {
        lfs fr0, 16(r11)
        mflr r0
        stw r0, -8(r1)
        stwu r1, -0x60(r1)
        stfd fr1, 0x50(r1)
        fmr fr1, fr0
        bl ScaleThunk
        fmr fr0, fr1
        lfd fr1, 0x50(r1)
        addi r1, r1, 0x60
        lwz r0, -8(r1)
        mtlr r0
        fmuls fr13, fr0, fr28
        lfs fr12, 16(r10)
        fsubs fr11, fr12, fr13
        lis r0, 0x8210
        ori r0, r0, 0x7E00
        mtctr r0
        bctr
    }
}

__declspec(naked) void RatioFovHook()
{
    __asm
    {
        lwz r11, -11348(r5)
        lfs fr0, 16(r11)
        mflr r0
        stw r0, -8(r1)
        stwu r1, -0x60(r1)
        stfd fr1, 0x50(r1)
        fmr fr1, fr0
        bl FovThunk
        fmr fr0, fr1
        lfd fr1, 0x50(r1)
        addi r1, r1, 0x60
        lwz r0, -8(r1)
        mtlr r0
        fmuls fr13, fr0, fr29
        fmuls fr1, fr13, fr28
        lis r0, 0x8210
        ori r0, r0, 0x7F10
        mtctr r0
        bctr
    }
}

__declspec(naked) void ProjectionFovHook()
{
    __asm
    {
        lwz r11, -11348(r10)
        lfs fr7, 16(r11)
        mflr r0
        stw r0, -8(r1)
        stwu r1, -0x60(r1)
        stfd fr1, 0x50(r1)
        fmr fr1, fr7
        bl FovThunk
        fmr fr7, fr1
        lfd fr1, 0x50(r1)
        addi r1, r1, 0x60
        lwz r0, -8(r1)
        mtlr r0
        fmuls fr6, fr7, fr0
        fmuls fr1, fr6, fr30
        lis r0, 0x820E
        ori r0, r0, 0x10AC
        mtctr r0
        bctr
    }
}

struct ReadHook
{
    uintptr_t address;
    void (*target)();
    uint32_t expected[4];
};

const ReadHook readHooks[] = {
    {0x82107C18, BaseFovHook, {0x3B8BF058, 0x817BD3AC, 0x7FA8E02E, 0xC38B0010}},
    {0x82107D00, RecoveryFovHook, {0xC9A10050, 0xFD60069C, 0xFD806E9C, 0xC14B0010}},
    {0x82107DF0, ScaleFovHook, {0xC00B0010, 0xEDA00732, 0xC18A0010, 0xED6C6828}},
    {0x82107F00, RatioFovHook, {0x8165D3AC, 0xC00B0010, 0xEDA00772, 0xEC2D0732}},
    {0x820E109C, ProjectionFovHook, {0x816AD3AC, 0xC0EB0010, 0xECC70032, 0xEC2607B2}},
};
} // namespace

void Fov::OnDvarInit()
{
    const auto registerFloat = reinterpret_cast<dvar_s *(*)(const char *, float, float, float, DvarFlags, const char *)>(0x8228C630);
    const auto archive = static_cast<DvarFlags>(1);
    userFov = registerFloat("cg_fov_user", 65.0f, 1.0f, 160.0f, archive, "Preferred base field of view");
    userScale = registerFloat("cg_fovscale_user", 1.0f, 0.2f, 2.0f, archive, "Preferred field of view scale");
}

Fov::Fov()
{
    // Check the complete patch set before changing any code.
    for (size_t i = 0; i < ARRAYSIZE(readHooks); ++i)
    {
        const auto &hook = readHooks[i];
        if (memcmp(reinterpret_cast<const void *>(hook.address), hook.expected, sizeof(hook.expected)) != 0)
        {
            DbgPrint("[codxe][T4 SP][FOV] Unexpected TU7 instructions at %08X; hooks not installed\n", hook.address);
            return;
        }
    }
    for (size_t i = 0; i < ARRAYSIZE(readHooks); ++i)
    {
        const auto &hook = readHooks[i];
        Detour::WriteFarBranch(reinterpret_cast<void *>(hook.address), hook.target, false);
        KeFlushCacheRange(reinterpret_cast<void *>(hook.address), sizeof(hook.expected));
    }
    hooksInstalled = true;
}

Fov::~Fov()
{
    if (!hooksInstalled)
        return;
    for (size_t i = 0; i < ARRAYSIZE(readHooks); ++i)
    {
        const auto &hook = readHooks[i];
        if (MmIsAddressValid(reinterpret_cast<void *>(hook.address)))
        {
            memcpy(reinterpret_cast<void *>(hook.address), hook.expected, sizeof(hook.expected));
            KeFlushCacheRange(reinterpret_cast<void *>(hook.address), sizeof(hook.expected));
        }
    }
    hooksInstalled = false;
}
} // namespace sp
} // namespace t4
