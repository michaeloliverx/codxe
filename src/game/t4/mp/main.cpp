#include "common.h"
#include "main.h"
#include "modules.h"

namespace t4
{
    namespace mp
    {
        void init()
        {
            register_modules();
        }

        void shutdown()
        {
            cleanup_modules();
        }
    }
}
