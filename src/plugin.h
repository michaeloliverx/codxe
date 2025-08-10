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
    virtual ~Plugin()
    {
        DbgPrint("Plugin::~Plugin\n");
    };

    /* Determines if this plugin should load. */
    static bool ShouldLoad();

    void RegisterModule(Module *module)
    {
        if (module)
        {
            DbgPrint("Plugin: Registering module: %s\n", module->get_name());
            m_modules.push_back(std::unique_ptr<Module>(module));
        }
    }

  private:
    std::vector<std::unique_ptr<Module>> m_modules;
};
