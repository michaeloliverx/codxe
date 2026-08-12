#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class command : public Module
{
  public:
    command();
    ~command();

    static void add(const char *name, void (*callback)());
    static void OnCmdInit();

  private:
    static cmd_function_s *allocate();
    static void register_all();
    static void register_command(cmd_function_s *command);

    static std::vector<cmd_function_s *> commands;
};
} // namespace mp
} // namespace iw3
