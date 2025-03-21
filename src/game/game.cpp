#include <xtl.h>

#include "../xboxkrnl.h"
#include "game.h"
#include "mp_main.h"
#include "sp_main.h"

namespace game
{
    uint32_t XBOX_360_TITLE_ID = 0x415607E6;

    void init()
    {
        if (strncmp((char *)0x82032AC4, "multiplayer", 11) == 0)
        {
            xbox::show_notification(L"IW3xe mp");
            mp::init();
        }
        else if (strncmp((char *)0x82065E48, "startSingleplayer", 17) == 0)
        {
            xbox::show_notification(L"IW3xe sp");
            sp::init();
        }
        else
        {
            xbox::show_notification(L"IW3xe unsupported executable");
        }
    }
}
