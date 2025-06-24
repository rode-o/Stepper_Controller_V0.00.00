/*  SFL3S-0600F.cpp  –  minimal flow-sensor driver
 *  ------------------------------------------------
 *  • uses SensirionI2cSf06Lf in H₂O continuous mode
 *  • applies ±error calibration and returns mL/min
 */

#include "../../../include/_include.hpp"          // State::getErrorPercent(), ENABLE_SFL3S_0600F
#include "SFL3S-0600F.hpp"
#include <Wire.h>
#include <SensirionI2cSf06Lf.h>

#ifdef ENABLE_SFL3S_0600F

/* ───── constants ─────────────────────────────────────────── */
static constexpr uint8_t  I2C_ADDR = SLF3S_0600F_I2C_ADDR_08;          // 0x08
static constexpr Sf06LfInvFlowScaleFactors INV_SCALE =
        INV_FLOW_SCALE_FACTORS_SLF3S_0600F;                            // enum value 10

/* ───── local state ───────────────────────────────────────── */
static SensirionI2cSf06Lf _flow;
static bool     _measuring     = false;
static int16_t  _driverErr     = 0;

static float    _rawFlow_uLmin = 0.0f;
static float    _rawTempC      = 0.0f;
static uint16_t _lastFlags     = 0;
static uint16_t _readCount     = 0;   // skip first few frames

/* ───── public API impl ───────────────────────────────────── */
bool startFlowMeasurement()
{
    Wire.begin();
    _flow.begin(Wire, I2C_ADDR);
    _flow.stopContinuousMeasurement();
    delay(5);

    _driverErr = _flow.startH2oContinuousMeasurement();
    _measuring = (_driverErr == 0);
    _readCount = 0;
    return _measuring;
}

bool stopFlowMeasurement()
{
    if (!_measuring) return true;
    _driverErr = _flow.stopContinuousMeasurement();
    _measuring = false;
    return (_driverErr == 0);
}

float readFlow()
{
    if (!_measuring) return 0.0f;

    int16_t err = _flow.readMeasurementData(
                      INV_SCALE,          // enum, not uint8_t
                      _rawFlow_uLmin,
                      _rawTempC,
                      _lastFlags);
    if (err) { _driverErr = err; return 0.0f; }

    // discard first 3 sensor frames for warm-up
    if (++_readCount <= 3) return 0.0f;

    /* scalar calibration factor from SystemState */
    float compFactor        = 1.0f /
                              (1.0f - State::getErrorPercent() / 100.0f);
    float compensated_uLmin = _rawFlow_uLmin * compFactor;

    return compensated_uLmin;
}

/* ───── simple accessors ──────────────────────────────────── */
float     getTempC()     { return _rawTempC;}          
uint16_t  getLastFlags() { return _lastFlags;}
float     getRawFlow()   { return _rawFlow_uLmin;}      // mL/min

#endif // ENABLE_SFL3S_0600F
