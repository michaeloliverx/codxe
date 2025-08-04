#include "g_scr_main.h"
#include "common.h"
#include <cstdio>

namespace iw4
{
    namespace sp
    {
        std::vector<BuiltinFunctionDef *> scr_functions;
        std::vector<BuiltinMethodDef *> scr_methods;

        void Scr_AddFunction(const char *name, BuiltinFunction func, int type)
        {
            BuiltinFunctionDef *newFunc = new BuiltinFunctionDef;
            newFunc->actionString = name;
            newFunc->actionFunc = func;
            newFunc->type = type;
            scr_functions.push_back(newFunc);
        }

        void Scr_AddMethod(const char *name, BuiltinMethod func, int type)
        {
            BuiltinMethodDef *newMethod = new BuiltinMethodDef;
            newMethod->actionString = name;
            newMethod->actionFunc = func;
            newMethod->type = type;
            scr_methods.push_back(newMethod);
        }

        Detour Scr_GetFunction_Detour;

        BuiltinFunction Scr_GetFunction_Hook(const char **pName, int *type)
        {
            if (pName != nullptr)
            {
                for (size_t i = 0; i < scr_functions.size(); ++i)
                {
                    if (std::strcmp(*pName, scr_functions[i]->actionString) == 0)
                    {
                        *type = scr_functions[i]->type;
                        return scr_functions[i]->actionFunc;
                    }
                }
            }
            else
            {
                for (size_t i = 0; i < scr_functions.size(); ++i)
                {
                    Scr_RegisterFunction(reinterpret_cast<int>(scr_functions[i]->actionFunc), scr_functions[i]->actionString);
                }
            }

            return Scr_GetFunction_Detour.GetOriginal<decltype(Scr_GetFunction)>()(pName, type);
        }

        Detour Scr_GetMethod_Detour;

        BuiltinMethod Scr_GetMethod_Hook(const char **pName, int *type)
        {
            if (pName != nullptr)
            {
                for (size_t i = 0; i < scr_methods.size(); ++i)
                {
                    if (std::strcmp(*pName, scr_methods[i]->actionString) == 0)
                    {
                        *type = scr_methods[i]->type;
                        return scr_methods[i]->actionFunc;
                    }
                }
            }
            else
            {
                for (size_t i = 0; i < scr_methods.size(); ++i)
                {
                    Scr_RegisterFunction(reinterpret_cast<int>(scr_methods[i]->actionFunc), scr_methods[i]->actionString);
                }
            }

            return Scr_GetMethod_Detour.GetOriginal<decltype(Scr_GetMethod)>()(pName, type);
        }

        void GScr_CbufAddText()
        {
            if (Scr_GetNumParam() != 1)
            {
                Scr_Error("Usage: exec(<string>)\n");
            }
            // VM strings are null-terminated, so no need to manually terminate
            // the string here.
            const char *text = Scr_GetString(0);
            Cbuf_AddText(0, text);
        }

        g_scr_main::g_scr_main()
        {
            Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
            Scr_GetFunction_Detour.Install();

            Scr_GetMethod_Detour = Detour(Scr_GetMethod, Scr_GetMethod_Hook);
            Scr_GetMethod_Detour.Install();

            Scr_AddFunction("exec", GScr_CbufAddText, 0);
        }

        g_scr_main::~g_scr_main()
        {
            Scr_GetFunction_Detour.Remove();

            Scr_GetMethod_Detour.Remove();
        }
    }
}
