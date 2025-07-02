#include "open_ctrl.hpp"
#include "../../main.hpp"
#include "../../devices/_devices.hpp"  // PumpDrv, STEPS_PER_REV
#include "../utils_ctrl/utils_ctrl.hpp"    // spsToTop, STEPS_PER_REV

using PumpDrv::stop;  // bring helper into scope

void OpenLoopCtrl::setup()
{
    PumpDrv::setTop(0);                    // idle on entry
}

void OpenLoopCtrl::loop()
{
    /* ---------- fail-safe guard ---------- */
    if (!State::isPumpEnabled()) {
        stop();                            // driver off
        State::setTop(0);
        State::setSPS(0);
        State::setRPM(0);
        return;
    }

    /* ---------- open-loop rate ---------- */
    float rpm = State::read().setRpm;
    float sps = rpm * STEPS_PER_REV / 60.0f;
    uint16_t top = spsToTop(sps);

    PumpDrv::setTop(top);
    State::setTop(top);
    State::setSPS(sps);
    State::setRPM(rpm);
}
