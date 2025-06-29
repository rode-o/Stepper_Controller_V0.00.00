#pragma once
#include <stdint.h>

/* ───── Calibration Timing Constants ───── */
constexpr uint32_t CAL_SETTLE_MS = 5'000;     // pump spin-up
constexpr uint32_t CAL_WINDOW_MS = 30'000;    // 30-s sample
constexpr uint32_t CAL_TOTAL_MS  = CAL_SETTLE_MS + CAL_WINDOW_MS;

/* ───── UI globals (defined ONCE in min_ctrl.cpp) ───── */
extern volatile bool     gCalibRunning;   // true while calibration blocks
extern volatile uint32_t gCalibStart;     // millis() timestamp
