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
    struct entry
    {
        std::string name;
        void (*callback)();
        cmd_function_s *function;
    };

    static cmd_function_s *allocate();
    static void register_all();
    static void register_command(entry *command);

    static std::vector<entry *> commands;
};
} // namespace sp
} // namespace iw3
