#pragma once

#include "pch.h"
#include <unordered_map>

namespace t4
{
namespace mp
{
// Keep the assembly bridge distinct from IW3's C-linkage GetReplacedPos symbol.
extern "C" const char *T4_GetReplacedPos(const char *pos, scriptInstance_t inst);

class GSCFunctions : public Module
{
  public:
    GSCFunctions();
    ~GSCFunctions();

    static void OnVMShutdown();
    static void ReplaceFunc();

  private:
    friend const char *T4_GetReplacedPos(const char *pos, scriptInstance_t inst);

    static std::unordered_map<const char *, const char *> ReplacedFunctions;

    static const char *GetCodePosForParam(int index);
    static const char *GetReplacedPos(const char *pos, scriptInstance_t inst);

    static Detour VM_Execute_Detour;
    static void VM_Execute_Hook();
};
} // namespace mp
} // namespace t4
