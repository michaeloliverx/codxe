#include "pch.h"
#include "symbols.h"

namespace t4
{
namespace mp
{

bool SV_IsClientBot(unsigned int clientNum)
{
    assert(clientNum < (*sv_maxclients)->current.unsignedInt);

    return svsHeader->clients[clientNum].header.netchan.remoteAddress.type == NA_BOT;
}

} // namespace mp
} // namespace t4
