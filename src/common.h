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

// Project-specific includes
#include "detour.h"
#include "filesystem.h"
#include "version.h"
#include "xboxkrnl.h"

// IW3-specific includes
#include "game/iw3/mp/main.h"
#include "game/iw3/mp/structs.h"
#include "game/iw3/mp/symbols.h"

#include "game/iw3/sp/main.h"
#include "game/iw3/sp/structs.h"

// T4-specific includes
#include "game/t4/mp/main.h"
#include "game/t4/mp/structs.h"
#include "game/t4/mp/symbols.h"

#include "game/t4/sp/components/test_module.h"
#include "game/t4/sp/main.h"
#include "game/t4/sp/structs.h"
#include "game/t4/sp/symbols.h"

/*
 * Usage:
 * Simply include this file in your source files:
 *
 * #include "common.h"
 *
 * This will include all commonly used headers for the project.
 */
