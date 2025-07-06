#pragma once
#include "../_ctrl.hpp"          // IController base, ControlMode enum

class ClosedLoopCtrl : public IController {
public:
    void setup() override;
    void loop () override;

private:
    double mMeas{0};   // PID input   (filtered flow)
    double mOut {0};   // PID output  (target flow uL/min)
    double mSet {0};   // PID set-point
};
