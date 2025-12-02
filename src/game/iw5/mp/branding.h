#pragma once

#include "pch.h"

class Branding : public Module
{
  public:
    Branding();
    ~Branding();

    const char *get_name() override
    {
        return "Branding";
    };
};
