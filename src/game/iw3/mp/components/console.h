#pragma once

#include "pch.h"

namespace iw3
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
    static void send_key_event(int key, bool down, unsigned int time);
    static void send_char_event(WCHAR unicode);
};

} // namespace mp
} // namespace iw3
