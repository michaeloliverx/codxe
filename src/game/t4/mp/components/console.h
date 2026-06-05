#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{

class console : public Module
{
  public:
    console();
    ~console();

    const char *get_name() override
    {
        return "console";
    }

    static void frame();
    static void toggle();
    static void close();

  private:
    static int convert_virtual_key(WORD virtual_key, WCHAR unicode);
};

} // namespace mp
} // namespace t4
