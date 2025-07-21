/*
This module is not used anywhere in the project.
It contains useful code snippets for debugging and development purposes.
*/

#include "common.h"

namespace t4
{
    namespace sp
    {

        void PrintClientFields()
        {
            DbgPrint("=== Client Fields ===\n");

            // Loop until we hit a null name (typical terminator for such arrays)
            for (int i = 0; client_fields[i].name != nullptr; i++)
            {
                const auto &field = client_fields[i];
                DbgPrint("[%d] Field: %s\n", i, field.name);
                DbgPrint("    Offset: 0x%X (%d)\n", field.ofs, field.ofs);
                DbgPrint("    Type: 0x%X\n", field.type);
                DbgPrint("    WhichBits: 0x%X\n", field.whichbits);
                DbgPrint("    Setter: 0x%p\n", field.setter);
                DbgPrint("    Getter: 0x%p\n", field.getter);
                DbgPrint("\n");
            }

            DbgPrint("=== End of Client Fields ===\n");
        }

        void PrintSpawnFunctions()
        {
            DbgPrint("=== Spawn Functions ===\n");

            for (int i = 0; i < BSP_ONLY_SPAWNS_COUNT; i++)
            {
                const SpawnFuncEntry &entry = s_bspOnlySpawns[i];
                DbgPrint("s_bspOnlySpawns[%d]: Classname: %s, Callback: %p\n", i, entry.classname, entry.callback);
            }
            for (int i = 0; i < BSP_OR_DYNAMIC_SPAWNS_COUNT; i++)
            {
                const SpawnFuncEntry &entry = s_bspOrDynamicSpawns[i];
                DbgPrint("s_bspOrDynamicSpawns[%d]: Classname: %s, Callback: %p\n", i, entry.classname, entry.callback);
            }

            DbgPrint("=== End of Spawn Functions ===\n");
        }
    }
}
