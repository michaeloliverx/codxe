#include "asset_dumper.h"
#include "common.h"

namespace t4
{
    namespace mp
    {
        AssetDumperModule::AssetDumperModule()
        {
            DbgPrint("AssetDumperModule initialized\n");
        }

        AssetDumperModule::~AssetDumperModule()
        {
            DbgPrint("AssetDumperModule shutdown\n");
        }
    }
}
