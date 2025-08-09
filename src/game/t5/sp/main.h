#pragma once

namespace t5
{
class Module
{
  public:
    Module() {};
    virtual ~Module() {};
    virtual const char *get_name()
    {
        return "unknown";
    }
};

namespace sp
{
void init();
void shutdown();
} // namespace sp
} // namespace t5
