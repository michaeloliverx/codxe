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
#include <xtl.h>
#include <xbox.h>
#include <xgraphics.h>

// Project-specific includes
#include "detour.h"
#include "filesystem.h"
#include "version.h"
#include "xboxkrnl.h"

// IW3-specific includes
#include "game/mp_structs.h"
#include "game/mp_gscr_fields.h"
#include "game/mp_main.h"
#include "game/sp_main.h"
#include "game/sp_structs.h"

// T4-specific includes
#include "game/t4/mp/main.h"
#include "game/t4/mp/structs.h"
#include "game/t4/mp/symbols.h"
#include "game/t4/sp/main.h"

/*
 * Usage:
 * Simply include this file in your source files:
 *
 * #include "common.h"
 *
 * This will include all commonly used headers for the project.
 */
