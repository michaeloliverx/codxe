#include "structs.h"
#include "cg_consolecmds.h"
#include "scr_parser.h"

namespace iw3_328
{
    void init()
    {
        init_cg_consolecmds();
        init_scr_parser();
    }
}
