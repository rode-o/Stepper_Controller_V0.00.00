#pragma once
/*  SFL3S-0600F.hpp  –  wrapper for Sensirion SLF3S-0600F
 *  All flow values are in µL·min⁻¹.
 *
 *  Functions (enabled when ENABLE_SFL3S_0600F is defined):
 *      startFlowMeasurement / stopFlowMeasurement
 *      readFlow()           -> compensated, µL·min⁻¹
 *      getTempC()           -> °C
 *      getLastFlags()       -> status bits
 *      getRawFlow()         -> un-compensated, µL·min⁻¹
 */

#include <stdint.h>

/* Provide the default I²C address only if the driver header lacks it */
#ifndef SLF3S_0600F_I2C_ADDR_08
#define SLF3S_0600F_I2C_ADDR_08 0x08
#endif

#ifdef ENABLE_SFL3S_0600F

bool      startFlowMeasurement();
bool      stopFlowMeasurement();

float     readFlow();           // µL·min⁻¹
float     getTempC();           // °C
uint16_t  getLastFlags();       // status bits
float     getRawFlow();         // µL·min⁻¹ (raw)

#endif /* ENABLE_SFL3S_0600F */
