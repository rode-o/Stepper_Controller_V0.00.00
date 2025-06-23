#pragma once
#include <Arduino.h>
#include <DRV8825.h>

/* ------- EEPROM layout (RP2040 “EEPROM” emulation) ------------------- */
constexpr int EEPROM_ADDR_RPM = 0;        // stores one float (4 bytes)

/* -------------------------------------------------------------------- */
class StepperController {
public:
    StepperController(uint8_t stepPin, uint8_t dirPin, uint8_t enPin);

    void  begin(uint16_t stepsPerRev, uint8_t microstep);

    void  enable()               { _drv.enable();  }
    void  disable()              { _drv.disable(); }

    void  setRPM(float rpmTarget);                 // ramps to new set-point
    void  runContinuous(bool on, bool cw = true);  // start / stop free-run
    void  moveRelative(long steps);                // queued move
    void  service();                               // call every loop()

    /* status */
    long  getPosition()  const { return _pos; }
    float getRPMTarget() const { return _rpmTarget; }
    float getRPMActual() const { return _rpmActual; }
    bool  isBusy()       const { return !_runForever && (_pos != _target); }

private:
    void  recomputeIntervals();

    DRV8825  _drv;
    uint8_t  _stepPin, _dirPin, _enPin;
    uint16_t _stepsPerRev = 200;
    uint8_t  _microstep   = 1;

    volatile long _target = 0;
    volatile long _pos    = 0;

    bool _runForever = false;
    bool _dirCont    = true;     // desired CW/CCW while running
    bool _prevDir    = true;     // DIR level last written to driver

    /* speed & accel */
    float    _rpmTarget   = 60.0f;     // user set-point (saved)
    float    _rpmActual   = 60.0f;     // ramps toward target
    uint32_t _intTargetUs = 0;         // µs per step at target rpm
    uint32_t _intCurrUs   = 0;         // current µs per step (accel)
    const    float _accelRpmPerSec = 300.0f;

    uint32_t _lastStep = 0;
};
