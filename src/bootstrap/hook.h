#pragma once
#include <xtl.h>
#include "../kernel.h"

// The resident hook owns no heap or CRT state.
struct ResidentHook
{
    bool Install(const xbox::KernelProfile &profile, const void *destination);
    void Remove();
    void *Original();
    bool IsInstalled() const
    {
        return source != NULL;
    }

  private:
    DWORD *source;
    DWORD saved[4];
};
