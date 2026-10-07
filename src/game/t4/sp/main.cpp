#include "pch.h"
#include "components/clipmap.h"
#include "components/console.h"
#include "components/debug.h"
#include "components/events.h"
#include "components/fastfiles.h"
#include "components/gsc_fields.h"
#include "components/gsc.h"
#include "components/scr_parser.h"
#include "components/splitscreen.h"
#include "components/ui.h"
#include "main.h"

namespace t4
{
namespace sp
{

Detour R_GetTextureFromCode_Detour;

uint32_t R_GetTextureFromCode_Hook(uint32_t source, unsigned int codeTexture, uint8_t *sampler)
{
    // TODO: track down the techset issue in the assets and remove this bandaid
    const auto image =
        R_GetTextureFromCode_Detour.GetOriginal<decltype(R_GetTextureFromCode)>()(source, codeTexture, sampler);
    if (image || codeTexture != 18)
        return image;

    // TU7 normally binds $white for dynamic shadows when shadow cookies are off. Some draws reach this
    // lookup before that binding, so supply the same fallback only when the slot is actually empty.
    const auto *scEnable = *reinterpret_cast<const uint8_t *const *>(0x84F1EFC4);
    if (!scEnable || scEnable[16] || !source)
        return image;

    const auto whiteImage = *reinterpret_cast<const uint32_t *>(0x84B58308);
    if (!whiteImage)
        return image;

    *reinterpret_cast<uint32_t *>(source + 4056) = whiteImage;
    static bool loggedFallback = false;
    if (!loggedFallback)
    {
        loggedFallback = true;
        DbgPrint("[codxe][T4 SP][Renderer] Bound $white to missing dynamic-shadow code image\n");
    }
    return whiteImage;
}

T4_SP_Plugin::T4_SP_Plugin()
{
#ifndef NDEBUG
    RegisterModule(new Debug());
#endif

    R_GetTextureFromCode_Detour = Detour(R_GetTextureFromCode, R_GetTextureFromCode_Hook);
    R_GetTextureFromCode_Detour.Install();

    // Default loc_warnings off to prevent console spam
    *(volatile uint8_t *)0x8225FA17 = 0x00;

    // Default ui_autoContinue on so level loads do not wait for input in solo or split-screen.
    *(volatile uint8_t *)0x82279B07 = 0x01;

    RegisterModule(new Config(Config::GAME_T4, true));
    RegisterModule(new Events()); // Must be registered before modules that subscribe to engine events.
    RegisterModule(new FastFiles());
    RegisterModule(new clipmap());
    RegisterModule(new console());
    RegisterModule(new GSC());
    RegisterModule(new GSCFields());
    RegisterModule(new scr_parser());
    RegisterModule(new SplitScreen());
    RegisterModule(new ui());
}

} // namespace sp
} // namespace t4
