#pragma once
/*  main.hpp – high-level application umbrella
 *  ------------------------------------------
 *  Includes every subsystem and exposes mainSetup / mainLoop
 *  for the tiny .ino wrapper.
 */

#include <Arduino.h>

#include "devices/_devices.hpp"   // all hardware drivers
#include "include/_include.hpp"
#include "core/_core.hpp"         // core helpers (filters, RGB, …)
#include "ctrl/_ctrl.hpp"         // interface + concrete controllers
#include "ctrl/main_ctrl/main_ctrl.hpp"   // ← NEW: orchestrator
//#include "utils/_utils.hpp"       // misc util libraries

/* entry points called from the .ino wrapper */
void setup();
void loop();
