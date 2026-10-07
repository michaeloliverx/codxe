#include "pch.h"
#include "components/scr_parser.h"
#include "main.h"

namespace iw2
{
namespace sp
{

IW2_SP_Plugin::IW2_SP_Plugin()
{
    RegisterModule(new Config(Config::GAME_IW2));
    RegisterModule(new scr_parser());
}

IW2_SP_Plugin::~IW2_SP_Plugin()
{
}

} // namespace sp
} // namespace iw2
