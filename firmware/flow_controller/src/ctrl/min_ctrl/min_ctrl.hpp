#pragma once
/*  min_ctrl.hpp – ultra-minimal control loop
    (only buttons + OLED page toggle for now)        */

#include <Arduino.h>

/* top-level entry points called from the .ino wrapper */
void ctrlSetup();
void ctrlLoop();
