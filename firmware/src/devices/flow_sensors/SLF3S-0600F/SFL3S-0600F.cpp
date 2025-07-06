/*  SFL3S-0600F.cpp  –  minimal flow-sensor driver
 *  Returns compensated flow in µL·min⁻¹.
 */

#include "../../../include/_include.hpp"          // State helpers
#include <Wire.h>
#include <SensirionI2cSf06Lf.h>                   // driver first
#include "SFL3S-0600F.hpp"

#ifdef ENABLE_SFL3S_0600F

/* ───── constants ───────────────────────────────────────── */
static constexpr uint8_t I2C_ADDR = SLF3S_0600F_I2C_ADDR_08;   // 0x08

/* Pick inverse-scale enum value (10) safely */
#ifdef INV_FLOW_SCALE_FACTORS_SLF3S_0600F
static constexpr Sf06LfInvFlowScaleFactors INV_SCALE =
        INV_FLOW_SCALE_FACTORS_SLF3S_0600F;      // enum constant
#else
static constexpr Sf06LfInvFlowScaleFactors INV_SCALE =
        static_cast<Sf06LfInvFlowScaleFactors>(10); // fallback
#endif

/* ───── local state ─────────────────────────────────────── */
static SensirionI2cSf06Lf _flow;
static bool     _measuring     = false;
static int16_t  _driverErr     = 0;

static float    _rawFlow_uLmin = 0.0f;
static float    _rawTempC      = 0.0f;
static uint16_t _lastFlags     = 0;
static uint16_t _readCount     = 0;      // skip first 3 frames

/* ───── API implementation ──────────────────────────────── */
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

float readFlow()   /* compensated, µL·min⁻¹ */
{
    if (!_measuring) return 0.0f;

    int16_t err = _flow.readMeasurementData(
                      INV_SCALE,
                      _rawFlow_uLmin,
                      _rawTempC,
                      _lastFlags);
    if (err) { _driverErr = err; return 0.0f; }

    if (++_readCount <= 3) return 0.0f;   // warm-up discard

    /* apply user ±cal-scalar (%) */
    float factor = 1.0f /
                   (1.0f - State::getCalScalar() / 100.0f);
    return _rawFlow_uLmin * factor;       // µL·min⁻¹
}

/* ───── simple accessors ───────────────────────────────── */
float     getTempC()     { return _rawTempC; }
uint16_t  getLastFlags() { return _lastFlags; }
float     getRawFlow()   { return _rawFlow_uLmin; }   // µL·min⁻¹

#endif /* ENABLE_SFL3S_0600F */
