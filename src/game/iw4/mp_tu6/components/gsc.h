#pragma once

#include "pch.h"
#include <unordered_map>

namespace iw4
{
namespace mp_tu6
{
// Keep the assembly bridge distinct from the IW3 C-linkage GetReplacedPos symbol.
extern "C" const char* IW4_GetReplacedPos(const char* pos);

class GSC : public Module
{
  public:
    GSC();
    ~GSC();

    static void OnVMShutdown();
    static void ReplaceFunc();

private:
    friend const char* IW4_GetReplacedPos(const char* pos);

    static std::unordered_map<const char*, const char*> ReplacedFunctions;

    static const char* GetCodePosForParam(int index);
    static const char* GetReplacedPos(const char* pos);

    static Detour VM_Execute_Detour;
    static void VM_Execute_Hook();
};
} // namespace mp_tu6
} // namespace iw4
