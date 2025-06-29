#pragma once
/* drv8825.hpp – 1/32-µstep pump driver for RP2040, AVR, fallback */

#include "../../../include/_include.hpp"   // pins, feature flags
#include <Arduino.h>

#ifdef ENABLE_DRV8825
namespace PumpDrv {

/* ---------- tuning constants -------------------------------- */
constexpr uint32_t MICROSTEP_DIV       = 32;        // 1/32-step
constexpr uint32_t MAX_FULL_SPS        = 1200;      // full-steps / s
constexpr uint32_t MAX_SPS             = MAX_FULL_SPS * MICROSTEP_DIV;
constexpr uint32_t MIN_SPS             = 20;
constexpr uint32_t ACCEL_SPS_PER_CYCLE = 0;         // 0 = no ramp

/* ---------- public API -------------------------------------- */
void  initPump();

/* speed-based interface (original) */
void  setTargetSPS(float sps);
void  setTargetRPM(float rpm);
void  pumpService();
float currentSPS();

/* period-driven interface (new, finer resolution) */
void  setTop(uint16_t top);             // 0 ⇒ stop / disable output

} // namespace PumpDrv
#endif
