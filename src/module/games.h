#pragma once
#include <xtl.h>

#define CODXE_STRINGIFY_INNER(value) #value
#define CODXE_STRINGIFY(value) CODXE_STRINGIFY_INNER(value)

namespace codxe
{
struct GameModuleInfo
{
    DWORD title;
    DWORD timestamp;
    const char *name;
    const char *filename;
    const char *directory;
    const char *description;
};

// Supported title IDs and executable timestamps. SP and MP share a data directory.
static const GameModuleInfo GameModules[] = {
    {0x415607D1u, 0x43E7A218u, "iw2_sp", "codxe.iw2.sp.xex", "iw2", "Call of Duty 2 SP TU3"},
    {0x415607D1u, 0x4456B8A3u, "iw2_mp", "codxe.iw2.mp.xex", "iw2", "Call of Duty 2 MP TU3"},
    {0x415607E1u, 0x45B56A5Cu, "ngl_mp", "codxe.ngl.mp.xex", "ngl", "Call of Duty 3 MP TU3"},
    {0x415607E6u, 0x46E1E82Fu, "iw3_sp", "codxe.iw3.sp.xex", "iw3", "Call of Duty 4 SP TU4"},
    {0x415607E6u, 0x4A78A577u, "iw3_mp", "codxe.iw3.mp.xex", "iw3", "Call of Duty 4 MP TU4"},
    {0x415607FFu, 0x48D0C2CAu, "qos_sp", "codxe.qos.sp.xex", "qos", "Quantum of Solace SP TU2"},
    {0x415607FFu, 0x49E8DEDEu, "qos_mp", "codxe.qos.mp.xex", "qos", "Quantum of Solace MP TU2"},
    {0x4156081Cu, 0x4AD7C0DFu, "t4_sp", "codxe.t4.sp.xex", "t4", "World at War SP TU7"},
    {0x4156081Cu, 0x4AD7C0EEu, "t4_mp", "codxe.t4.mp.xex", "t4", "World at War MP TU7"},
    {0x41560817u, 0x5074B056u, "iw4_sp", "codxe.iw4.sp.xex", "iw4", "Modern Warfare 2 SP TU9"},
    {0x41560817u, 0x5B11C269u, "iw4_mp", "codxe.iw4.mp.xex", "iw4", "Modern Warfare 2 MP TU9"},
    {0x41560817u, 0x4BE22338u, "iw4_mp_tu6", "codxe.iw4.mp.tu6.xex", "iw4", "Modern Warfare 2 MP TU6"},
    {0x41560855u, 0x4E542875u, "t5_sp", "codxe.t5.sp.xex", "t5", "Black Ops SP TU11"},
    {0x41560855u, 0x4E542876u, "t5_mp", "codxe.t5.mp.xex", "t5", "Black Ops MP TU11"},
    {0x415608CBu, 0x5B10A113u, "iw5_mp", "codxe.iw5.mp.xex", "iw5", "Modern Warfare 3 MP TU24"},
};

inline const GameModuleInfo *FindGameModule(DWORD title, DWORD timestamp)
{
    for (unsigned i = 0; i < sizeof(GameModules) / sizeof(GameModules[0]); ++i)
        if (GameModules[i].title == title && GameModules[i].timestamp == timestamp)
            return &GameModules[i];
    return 0;
}

inline const GameModuleInfo *FindGameModuleByName(const char *name)
{
    for (unsigned i = 0; i < sizeof(GameModules) / sizeof(GameModules[0]); ++i)
    {
        const char *expected = name;
        const char *actual = GameModules[i].name;
        while (*expected && *expected == *actual)
        {
            ++expected;
            ++actual;
        }
        if (*expected == *actual)
            return &GameModules[i];
    }
    return 0;
}
} // namespace codxe
