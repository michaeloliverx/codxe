#pragma once

#include "pch.h"

namespace iw2
{
namespace mp
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
} // namespace mp
} // namespace iw2
