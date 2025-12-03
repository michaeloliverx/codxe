#pragma once

#include "config.h"
namespace branding
{
enum Game
{
    GAME_IW3,
    GAME_IW4,
    GAME_IW5,
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
    case GAME_IW5:
        gameName = "IW5";
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

    if (Config::active_mod.empty())
    {
        _snprintf_s(brandingBuffer, sizeof(brandingBuffer), "CoDxe - %s %s\nBuild: " __DATE__ " " __TIME__, gameName,
                    modeName);
    }
    else
    {
        _snprintf_s(brandingBuffer, sizeof(brandingBuffer),
                    "CoDxe - %s %s\nBuild: " __DATE__ " " __TIME__ "\nActive mod: %s", gameName, modeName,
                    Config::active_mod.c_str());
    }

    return brandingBuffer;
}
} // namespace branding
