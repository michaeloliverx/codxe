#include "gsc_functions.h"
#include "common.h"

namespace t4
{
    namespace mp
    {
        void GScr_TestMessage()
        {
            CG_GameMessage(0, "Hello from GSCFunctions!");
        }

        Detour Scr_GetFunction_Detour;

        BuiltinFunction Scr_GetFunction_Hook(const char **pName, int *type)
        {
            if (pName != nullptr)
            {
                if (std::strcmp(*pName, "testmessage") == 0)
                {
                    return reinterpret_cast<BuiltinFunction>(&GScr_TestMessage);
                }
            }

            return Scr_GetFunction_Detour.GetOriginal<decltype(&Scr_GetFunction_Hook)>()(pName, type);
        }

        GSCFunctions::GSCFunctions()
        {
            Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
            Scr_GetFunction_Detour.Install();
        }

        GSCFunctions::~GSCFunctions()
        {
            Scr_GetFunction_Detour.Remove();
        }
    }
}
