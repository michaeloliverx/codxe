#pragma once

#pragma warning(disable : 4201) // warning C4201: nonstandard extension used : nameless struct/union
#pragma warning(disable : 4324) // structure was padded due to __declspec(align())
#pragma warning(disable : 4480) // nonstandard extension used: specifying underlying type for enum

// Standard library includes
#include <array>
#include <climits>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <direct.h>
#include <fstream>
#include <sstream>
#include <xtl.h>
#include <xbox.h>
#include <xgraphics.h>
#include <io.h>
#include <xjson.h>
#include <rtcapi.h>
#include <xkelib.h>

#pragma comment(lib, "xjson.lib")

// Project-specific includes
#include "detour.h"
#include "filesystem.h"
#include "version.h"
#include "xbox360.h"
#include "plugin.h"
#include "compat.h"

// Common includes
#include "common/branding.h"
#include "common/config.h"
#include "common/map_ents.h"
#include "common/ppc.h"

#if defined(CODXE_GAME_IW2_SP)
#include "game/iw2/sp/main.h"
#include "game/iw2/sp/structs.h"
#include "game/iw2/sp/symbols.h"
#elif defined(CODXE_GAME_IW2_MP)
#include "game/iw2/mp/main.h"
#include "game/iw2/mp/structs.h"
#include "game/iw2/mp/symbols.h"
#elif defined(CODXE_GAME_NGL_MP)
#include "game/ngl/mp/main.h"
#include "game/ngl/mp/structs.h"
#include "game/ngl/mp/symbols.h"
#elif defined(CODXE_GAME_IW3_SP)
#include "game/iw3/sp/main.h"
#include "game/iw3/sp/structs.h"
#include "game/iw3/sp/symbols.h"
#elif defined(CODXE_GAME_IW3_MP)
#include "game/iw3/mp/main.h"
#include "game/iw3/mp/structs.h"
#include "game/iw3/mp/symbols.h"
#elif defined(CODXE_GAME_QOS_SP)
#include "game/qos/sp/main.h"
#include "game/qos/sp/structs.h"
#include "game/qos/sp/symbols.h"
#elif defined(CODXE_GAME_QOS_MP)
#include "game/qos/mp/main.h"
#include "game/qos/mp/structs.h"
#include "game/qos/mp/symbols.h"
#elif defined(CODXE_GAME_T4_SP)
#include "game/t4/sp/main.h"
#include "game/t4/sp/structs.h"
#include "game/t4/sp/symbols.h"
#elif defined(CODXE_GAME_T4_MP)
#include "game/t4/mp/main.h"
#include "game/t4/mp/structs.h"
#include "game/t4/mp/symbols.h"
#elif defined(CODXE_GAME_IW4_SP)
#include "game/iw4/sp/main.h"
#include "game/iw4/sp/structs.h"
#include "game/iw4/sp/symbols.h"
#elif defined(CODXE_GAME_IW4_MP)
#include "game/iw4/mp/main.h"
#include "game/iw4/mp/structs.h"
#include "game/iw4/mp/symbols.h"
#elif defined(CODXE_GAME_IW4_MP_TU6)
#include "game/iw4/mp_tu6/main.h"
#include "game/iw4/mp_tu6/structs.h"
#include "game/iw4/mp_tu6/symbols.h"
#elif defined(CODXE_GAME_T5_SP)
#include "game/t5/sp/main.h"
#include "game/t5/sp/structs.h"
#include "game/t5/sp/symbols.h"
#elif defined(CODXE_GAME_T5_MP)
#include "game/t5/mp/main.h"
#include "game/t5/mp/structs.h"
#include "game/t5/mp/symbols.h"
#elif defined(CODXE_GAME_IW5_MP)
#include "game/iw5/mp/main.h"
#include "game/iw5/mp/structs.h"
#include "game/iw5/mp/symbols.h"
#else
#error Select a game module project.
#endif
