#pragma once

// Standard library includes
#include <cstddef>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <algorithm>
#include <functional>
#include <cstdint>
#include <cstring>
#include <direct.h>
#include <fstream>
#include <sstream>
#include <xtl.h>
#include <xbox.h>
#include <xgraphics.h>
#include <io.h>
#include <xjson.h>
using namespace std::tr1;

// Project-specific includes
#include "detour.h"
#include "filesystem.h"
#include "version.h"
#include "xboxkrnl.h"
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

/*
 * Usage:
 * Simply include this file in your source files:
 *
 * #include "common.h"
 *
 * This will include all commonly used headers for the project.
 */
