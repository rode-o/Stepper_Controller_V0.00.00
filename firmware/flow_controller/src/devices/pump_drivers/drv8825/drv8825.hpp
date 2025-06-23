#pragma once
/* drv8825.hpp – 1/32-µstep pump driver (RP2040)               */
/* ----------------------------------------------------------- */

#include "../../../include/_include.hpp"   // pins.hpp / ENABLE_DRV8825
#include <Arduino.h>

#ifdef ENABLE_DRV8825

namespace PumpDrv {

/* ---------- tuneables ------------------------------------- */
constexpr uint32_t MICROSTEP_DIV         = 32;       // 1/32
constexpr uint32_t MAX_FULL_SPS          = 1200;     // full-steps s-1
constexpr uint32_t MAX_SPS               = MAX_FULL_SPS * MICROSTEP_DIV;
constexpr uint32_t MIN_SPS               = 20;
constexpr uint32_t ACCEL_SPS_PER_CYCLE   = 0;        // 0 = no ramp

/* ---------- public API ------------------------------------ */
void  initPump();

void  setTargetSPS(float sps);   // low-level
void  setTargetRPM(float rpm);   // high-level helper

void  pumpService();             // call every loop
float currentSPS();              // for logging

} // namespace PumpDrv
#endif  // ENABLE_DRV8825
