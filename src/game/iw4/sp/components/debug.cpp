#include "common.h"

namespace iw4
{
namespace sp
{

void DebugPrintClientFields()
{
    DbgPrint("=== Client Fields ===\n");

    // Loop until we hit a null name (typical terminator for such arrays)
    for (int i = 0; fields[i].name != nullptr; i++)
    {
        DbgPrint("[%d] Field: %s\n", i, fields[i].name);
        DbgPrint("    Offset: 0x%X (%d)\n", fields[i].ofs, fields[i].ofs);
        DbgPrint("    Type: 0x%X\n", fields[i].type);
        DbgPrint("    Setter: 0x%p\n", fields[i].setter);
        DbgPrint("    Getter: 0x%p\n", fields[i].getter);
        DbgPrint("\n");
    }

    DbgPrint("=== End of Client Fields ===\n");
}

// Compact table format
void PrintBuiltinMethodsTable()
{
    // static auto methods_1 = reinterpret_cast<BuiltinMethodDef *>(0x8201CA80);

    DbgPrint("=== Builtin Methods Table ===\n");
    DbgPrint("%-30s | %-10s | %s\n", "Method Name", "Function", "Type");
    DbgPrint("%-30s-+-%-10s-+-%s\n", "------------------------------", "----------", "----");

    for (int i = 0; i < METHODS_1_COUNT; i++)
    {
        DbgPrint("%-30s | 0x%08X | %d\n", methods_1[i].actionString, (uintptr_t)methods_1[i].actionFunc,
                 methods_1[i].type);
    }

    DbgPrint("=== End of Builtin Methods Table ===\n");
}
} // namespace sp
} // namespace iw4
