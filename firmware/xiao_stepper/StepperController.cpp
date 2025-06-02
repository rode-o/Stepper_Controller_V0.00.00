#include "StepperController.h"
#include <EEPROM.h>
#include <math.h>          // isfinite()

/* ------------------------------------------------------------------ */
/*  Constructor                                                       */
/* ------------------------------------------------------------------ */
StepperController::StepperController(uint8_t stepPin,
                                     uint8_t dirPin,
                                     uint8_t enPin)
: _drv(),
  _stepPin(stepPin), _dirPin(dirPin), _enPin(enPin),
  _prevDir(true)                // initialise last-dir to CW (HIGH)
{}

/* ------------------------------------------------------------------ */
/*  begin() – init driver and load saved RPM                          */
/* ------------------------------------------------------------------ */
void StepperController::begin(uint16_t stepsPerRev, uint8_t microstep)
{
    _stepsPerRev = stepsPerRev;
    _microstep   = microstep;

    _drv.begin(_dirPin, _stepPin, _enPin, 255, 255);
    _drv.enable();

    /* load last RPM from EEPROM (4-byte float) */
    EEPROM.begin(sizeof(float));
    float tmp;
    EEPROM.get(EEPROM_ADDR_RPM, tmp);
    if (!isfinite(tmp) || tmp < 1.0f || tmp > 1200.0f) tmp = 60.0f;

    _rpmTarget = _rpmActual = tmp;
    recomputeIntervals();
}

/* ------------------------------------------------------------------ */
void StepperController::recomputeIntervals()
{
    float µsPerStep = 60.0f * 1e6f / (_rpmTarget * _stepsPerRev * _microstep);
    _intTargetUs    = (uint32_t)µsPerStep;
    if (_intTargetUs == 0) _intTargetUs = 1;
}

/* ------------------------------------------------------------------ */
void StepperController::setRPM(float rpmTarget)
{
    _rpmTarget = constrain(rpmTarget, 1.0f, 1200.0f);
    recomputeIntervals();
    EEPROM.put(EEPROM_ADDR_RPM, _rpmTarget);
    EEPROM.commit();
}

/* ------------------------------------------------------------------ */
void StepperController::runContinuous(bool on, bool cw)
{
    _runForever = on;
    _dirCont    = cw;
}

/* ------------------------------------------------------------------ */
void StepperController::moveRelative(long steps) { _target += steps; }

/* ------------------------------------------------------------------ */
/*  service()  – non-blocking step engine                             */
/* ------------------------------------------------------------------ */
void StepperController::service()
{
    uint32_t now = micros();

    /* ---- ramp _rpmActual toward _rpmTarget ---- */
    static uint32_t lastAccel = now;
    float dt = (now - lastAccel) / 1e6f;           // sec
    lastAccel = now;

    float d = _rpmTarget - _rpmActual;
    float step = _accelRpmPerSec * dt;
    if (fabs(d) < step) _rpmActual = _rpmTarget;
    else _rpmActual += (d > 0 ? step : -step);

    float µsPerStep = 60.0f * 1e6f / (_rpmActual * _stepsPerRev * _microstep);
    _intCurrUs      = (uint32_t)µsPerStep;
    if (_intCurrUs == 0) _intCurrUs = 1;

    /* ---- need a step? ---- */
    bool need = _runForever || (_pos != _target);
    if (!need) return;
    if (now - _lastStep < _intCurrUs) return;

    /* ---- choose direction ---- */
    bool dir = _runForever ? _dirCont : (_target > _pos);

    /* ---- soft reverse if direction flips ---- */
    static const uint16_t DIR_PAUSE_MS = 5;   // coast interval
    if (dir != _prevDir) {
        _drv.setDirection(dir);               // toggle DIR pin
        _prevDir   = dir;

        _rpmActual = 0.0f;                    // decel to stand-still
        delay(DIR_PAUSE_MS);                  // small pause for rotor
        _lastStep = micros();                 // reset timer after pause
        return;                               // skip stepping this cycle
    }

    /* ---- issue STEP pulse ---- */
    _drv.step();
    _pos += dir ? 1 : -1;
    _lastStep = micros();
}
