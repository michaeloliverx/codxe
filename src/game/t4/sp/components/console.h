#pragma once

#include "pch.h"

namespace t4
{
namespace sp
{
class console : public Module
{
  public:
    console();
    ~console();

    static void OnUIRefresh();
    static void frame();
    static void toggle();
    static void close();

  private:
    static int convert_virtual_key(WORD virtual_key, WCHAR unicode);
};
} // namespace sp
} // namespace t4
