#pragma once

#include <cstdio>
#include "version.h"

namespace branding
{
    enum Game
    {
        GAME_IW3,
        GAME_IW4,
        GAME_T4
    };

    enum Mode
    {
        MODE_SP,
        MODE_MP
    };

    inline const char *GetBrandingString(Game game, Mode mode)
    {
        static char brandingBuffer[256];

        const char *gameName = "";
        const char *modeName = "";

        switch (game)
        {
        case GAME_IW3:
            gameName = "IW3";
            break;
        case GAME_IW4:
            gameName = "IW4";
            break;
        case GAME_T4:
            gameName = "T4";
            break;
        }

        switch (mode)
        {
        case MODE_SP:
            modeName = "SP";
            break;
        case MODE_MP:
            modeName = "MP";
            break;
        }

        _snprintf_s(brandingBuffer, sizeof(brandingBuffer),
                    "CODXE - %s %s " __DATE__ " " __TIME__,
                    gameName, modeName);

        return brandingBuffer;
    }
}
