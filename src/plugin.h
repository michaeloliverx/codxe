#pragma once

#include <memory>
#include <vector>

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

class Plugin
{
  public:
    Plugin() {};
    virtual ~Plugin() {};

    void RegisterModule(Module *module)
    {
        if (module)
        {
            m_modules.push_back(std::unique_ptr<Module>(module));
        }
    }

  private:
    std::vector<std::unique_ptr<Module>> m_modules;
};
