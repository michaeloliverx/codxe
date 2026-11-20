#include "pch.h"
#include "detour.h"

#pragma section(".text")
__declspec(allocate(".text")) BYTE Detour::TrampolineBuffer[8 * 1024] = {};
SIZE_T Detour::TrampolineSize = 0;
