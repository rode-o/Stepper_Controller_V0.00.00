#include "closed_ctrl.hpp"
#include "../../main.hpp"                  // State helpers
#include "../../devices/_devices.hpp"      // PumpDrv, STEPS_PER_REV
#include "../utils_ctrl/utils_ctrl.hpp"    // rateToTop(), topToSps()
#include <PID_v1.h>

using PumpDrv::stop;    // bring helper into scope

/* local PID instance */
static PID gPid(nullptr, nullptr, nullptr, 0.25, 0.125, 0.125, DIRECT);

void ClosedLoopCtrl::setup()
{
    mMeas = 0;
    mOut  = 0;
    mSet  = State::read().setFlow_uLmin;

    gPid = PID(&mMeas, &mOut, &mSet, 0.25, 0.125, 0.125, DIRECT);
    gPid.SetOutputLimits(0, 2000);   // µL/min
    gPid.SetSampleTime(100);         // 10 Hz
    gPid.SetMode(AUTOMATIC);
}

void ClosedLoopCtrl::loop()
{
    /* ---------- fail-safe guard ---------- */
    if (!State::isPumpEnabled()) {
        stop();                            // disable driver
        State::setTop(0);
        State::setSPS(0);
        State::setRPM(0);
        return;
    }

    /* ---------- PID control ---------- */
    mMeas = State::read().f_flow;          // filtered flow
    mSet  = State::read().setFlow_uLmin;   // target

    gPid.Compute();                        // updates mOut (uL/min)

    uint16_t top = rateToTop(mOut);        // flow → PWM TOP
    PumpDrv::setTop(top);
    State::setTop(top);

    float sps = topToSps(top);
    State::setSPS(sps);
    State::setRPM(sps * 60.0f / STEPS_PER_REV);
}
