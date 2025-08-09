#pragma once

#include "common.h"

namespace iw3
{
namespace mp
{
class scr_vm_functions : public Module
{
  public:
    scr_vm_functions();
    ~scr_vm_functions();

    const char *get_name() override
    {
        return "scr_vm_functions";
    };
};
} // namespace mp
} // namespace iw3
