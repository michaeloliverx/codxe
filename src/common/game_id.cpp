#include "pch.h"
#include "game_id.h"

namespace GameId
{
const char *ToString(Type gameId)
{
    switch (gameId)
    {
    case IW2:
        return "iw2";
    case IW3:
        return "iw3";
    case IW4:
        return "iw4";
    case IW5:
        return "iw5";
    case T4:
        return "t4";
    case T5:
        return "t5";
    case QOS:
        return "qos";
    default:
        return nullptr;
    }
}
} // namespace GameId
