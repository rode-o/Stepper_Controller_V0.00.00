#pragma once
/*  SFL3S-0600F.hpp  –  minimal driver wrapper
 *  -------------------------------------------------
 *  Public API (always returns / accepts mL·min⁻¹):
 *      bool  startFlowMeasurement();
 *      bool  stopFlowMeasurement();
 *      float readFlow();          // compensated flow  (mL/min)
 *      float getTempC();          // last °C
 *      uint16_t getLastFlags();   // status bits
 *      float getRawFlow();        // raw, no calibration (mL/min)
 *
 *  Requires:
 *      • SensirionI2cSf06Lf  (driver)
 *      • getErrorPercent()   (user calibration)  from _include.hpp
 *
 *  Compile this file only when ENABLE_SFL3S_0600F is defined.
 */

#include <stdint.h>

#ifdef ENABLE_SFL3S_0600F

/* ───── public API ─────────────────────────────────────────── */
bool      startFlowMeasurement();
bool      stopFlowMeasurement();
float     readFlow();           // compensated, mL/min
float     getTempC();           // °C
uint16_t  getLastFlags();       // raw status flags
float     getRawFlow();         // un-compensated, mL/min

#endif // ENABLE_SFL3S_0600F
