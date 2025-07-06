//fw_version.hpp

#pragma once

#define FW_VERSION_MAJOR 1
#define FW_VERSION_MINOR 0
#define FW_VERSION_PATCH 3

// ───── Stringification Macros ─────
#define _STR(x) #x
#define STR(x) _STR(x)

// ───── Auto-Generated Version String ─────
#define FW_VERSION_STRING "v" STR(FW_VERSION_MAJOR) "." STR(FW_VERSION_MINOR) "." STR(FW_VERSION_PATCH)
