#pragma once
#include "../../main.hpp"

/*  utils_ctrl.hpp – math, filters & volume helpers shared by all controllers  */

constexpr double PWM_CLKDIV   = 64.0;                     // slice divider
constexpr float  STEPS_PER_REV = 200.0f * PumpDrv::MICROSTEP_DIV;

/* wrap / rate helpers */
uint16_t rateToTop(double uLmin);
float    topToSps(uint16_t top);
uint16_t spsToTop(float sps);

/* sensor + bookkeeping */
float readAndFilterFlow();                   // returns filtered flow, pushes raw
void  updateVolume(float flow_uLmin, uint32_t dtMs);
