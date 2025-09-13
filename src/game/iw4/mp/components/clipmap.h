#pragma once

#include "common.h"

class clipmap : public Module
{
  public:
    clipmap();
    ~clipmap();

    const char *get_name() override
    {
        return "clipmap";
    };
};
