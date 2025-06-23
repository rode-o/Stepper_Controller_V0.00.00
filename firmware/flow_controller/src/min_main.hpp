#pragma once
/*  min_main.hpp – super-minimal application entry     */

#include <Arduino.h>
#include "devices/_devices.hpp"  // includes all devices
#include "include/_include.hpp"
#include "core/_core.hpp"        // includes all core modules
#include "ctrl/_ctrl.hpp"        // includes all ctrl modules
#include "utils/_utils.hpp"      // includes all utils modules


/* entry points called from the .ino wrapper */
void mainSetup();
void mainLoop();
