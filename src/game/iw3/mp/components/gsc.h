#pragma once

#include "pch.h"
#include <unordered_map>

namespace iw3
{
namespace mp
{
// The Xbox PowerPC inline assembler requires an unmangled name for its direct branch.
extern "C" const char *GetReplacedPos(const char *pos);

class GSC : public Module
{
  public:
    GSC();
    ~GSC();

    static void OnVMShutdown();
    static void ReplaceFunc();

  private:
    friend const char *GetReplacedPos(const char *pos);

    static std::unordered_map<const char *, const char *> ReplacedFunctions;

    static const char *GetCodePosForParam(int index);
    static const char *GetReplacedPos(const char *pos);

    static Detour VM_Execute_Detour;
    static void VM_Execute_Hook();
};
} // namespace mp
} // namespace iw3
