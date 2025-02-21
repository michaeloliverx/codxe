#include "detour.h"

#pragma section(".text")
__declspec(allocate(".text")) BYTE Detour::TrampolineBuffer[200 * 20] = {};
SIZE_T Detour::TrampolineSize = 0;
