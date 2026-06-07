#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
void Scr_AddMethod(const char *name, BuiltinMethod func, scr_builtin_type_t type);
void Scr_AddFunction(const char *name, BuiltinFunction func, scr_builtin_type_t type);

class g_scr_main : public Module
{
  public:
    g_scr_main();
    ~g_scr_main();
    const char *get_name() override
    {
        return "g_scr_main";
    };
};
} // namespace mp_tu6
} // namespace iw4
