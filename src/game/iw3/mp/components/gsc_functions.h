#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
void GScr_CbufAddText();
void GSCrGetPlayerclipBrushesContainingPoint();
void GScr_FS_TestFile();
void GScr_FS_FOpen();
void GScr_FS_FClose();
void GScr_FS_ReadLine();
void GScr_FS_WriteLine();
void Scr_IsArray_f();
void GScr_Float();

class gsc_functions : public Module
{
  public:
    gsc_functions();
    ~gsc_functions();

    const char *get_name() override
    {
        return "gsc_functions";
    };
};
} // namespace mp
} // namespace iw3
