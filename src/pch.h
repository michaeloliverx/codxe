#pragma once

// Standard library includes
#include <array>
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

#pragma comment(lib, "d3d9ltcg.lib")
#pragma comment(lib, "xgraphics.lib")
#pragma comment(lib, "xjson.lib")

// Project-specific includes
#include "detour.h"
#include "filesystem.h"
#include "version.h"
#include "xkelib.h"
#include "xbox360.h"
#include "plugin.h"

// Common includes
#include "common/branding.h"
#include "common/config.h"

// IW2-specific includes
#include "game/iw2/sp/main.h"
#include "game/iw2/sp/structs.h"
#include "game/iw2/sp/symbols.h"

#include "game/iw2/mp/main.h"
#include "game/iw2/mp/structs.h"
#include "game/iw2/mp/symbols.h"

// IW3-specific includes
#include "game/iw3/mp/main.h"
#include "game/iw3/mp/structs.h"
#include "game/iw3/mp/symbols.h"

#include "game/iw3/sp/main.h"
#include "game/iw3/sp/structs.h"
#include "game/iw3/sp/symbols.h"

// IW4-specific includes
#include "game/iw4/sp/main.h"
#include "game/iw4/sp/structs.h"
#include "game/iw4/sp/symbols.h"

#include "game/iw4/mp/main.h"
#include "game/iw4/mp/structs.h"
#include "game/iw4/mp/symbols.h"

// IW5-specific includes
#include "game/iw5/mp/main.h"
#include "game/iw5/mp/structs.h"
#include "game/iw5/mp/symbols.h"

// T4-specific includes
#include "game/t4/mp/main.h"
#include "game/t4/mp/structs.h"
#include "game/t4/mp/symbols.h"

#include "game/t4/sp/main.h"
#include "game/t4/sp/structs.h"
#include "game/t4/sp/symbols.h"

// T5-specific includes
#include "game/t5/mp/main.h"
#include "game/t5/mp/structs.h"
#include "game/t5/mp/symbols.h"

#include "game/t5/sp/main.h"
#include "game/t5/sp/structs.h"
#include "game/t5/sp/symbols.h"

// QOS-specific includes
#include "game/qos/sp/main.h"
#include "game/qos/sp/structs.h"
#include "game/qos/sp/symbols.h"

#include "game/qos/mp/main.h"
#include "game/qos/mp/structs.h"
#include "game/qos/mp/symbols.h"
