#pragma once

#include "pch.h"

namespace t4
{
namespace sp
{
void ScriptEntCmd_CloneBrushModelToScriptModel(scr_entref_t entref);

class g_scr_mover : public Module
{
  public:
    g_scr_mover();
    ~g_scr_mover();

    const char *get_name() override
    {
        return "g_scr_mover";
    };
};
} // namespace sp
} // namespace t4
