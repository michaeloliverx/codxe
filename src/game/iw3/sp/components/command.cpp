#include "pch.h"
#include "command.h"
#include "events.h"

namespace iw3
{
namespace sp
{
std::vector<cmd_function_s *> command::commands;

command::command()
{
    Events::OnCmdInit([] { register_all(); });
}

command::~command()
{
    for (size_t i = 0; i < commands.size(); ++i)
    {
        delete commands[i];
    }

    commands.clear();
}

void command::add(const char *name, void (*callback)())
{
    cmd_function_s *new_command = allocate();
    new_command->name = name;
    new_command->function = reinterpret_cast<void (*)()>(callback);

    commands.push_back(new_command);
}

cmd_function_s *command::allocate()
{
    cmd_function_s *cmd = new cmd_function_s;
    ZeroMemory(cmd, sizeof(cmd_function_s));
    return cmd;
}

void command::register_all()
{
    for (size_t i = 0; i < commands.size(); ++i)
    {
        register_command(commands[i]);
    }
}

void command::register_command(cmd_function_s *command)
{
    Cmd_AddCommandInternal(command->name, reinterpret_cast<void (*)()>(command->function), command);
}
} // namespace sp
} // namespace iw3
