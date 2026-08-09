#include "pch.h"
#include "components/clipmap.h"
#include "components/gsc_fields.h"
#include "components/gsc.h"
#include "components/scr_parser.h"
#include "components/ui.h"
#include "main.h"

namespace t4
{
namespace sp
{

T4_SP_Plugin::T4_SP_Plugin()
{
    RegisterModule(new Config());
    RegisterModule(new clipmap());
    RegisterModule(new GSC());
    RegisterModule(new GSCFields());
    RegisterModule(new scr_parser());
    RegisterModule(new ui());
}

} // namespace sp
} // namespace t4
