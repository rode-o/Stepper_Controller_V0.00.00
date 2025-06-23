#pragma once
/* rgb.hpp – tiny wrapper for the on-board WS2812 (XIAO RP2040)
   Requires:  Adafruit_NeoPixel library
   ─────────────────────────────────────────────────────────── */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "../../include/_include.hpp"  // LEDColour

namespace RGB {

/* call once in setup() ------------------------------------------------ */
void begin();

/* set solid colour instantly ----------------------------------------- */
void setColour(LEDColour c);

}   // namespace RGB
