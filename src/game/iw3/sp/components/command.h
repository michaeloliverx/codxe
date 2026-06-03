#pragma once

#include "pch.h"

namespace iw3
{
namespace sp
{
class command : public Module
{
  public:
    command();
    ~command();

    const char *get_name() override
    {
        return "command";
    };

    static void add(const char *name, void (*callback)());

  private:
    static cmd_function_s *allocate();
    static void register_all();
    static void register_command(cmd_function_s *command);

    static std::vector<cmd_function_s *> commands;
};
} // namespace sp
} // namespace iw3
