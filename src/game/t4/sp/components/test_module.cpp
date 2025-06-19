#include "test_module.h"
#include "common.h"

namespace t4
{
    namespace sp
    {
        TestModule::TestModule()
        {
            DbgPrint("TestModule initialized\n");
        }

        TestModule::~TestModule()
        {
            DbgPrint("TestModule shutdown\n");
        }
    }
}
