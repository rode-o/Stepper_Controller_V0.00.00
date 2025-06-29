#pragma once
#include "egc_types.hpp"
#include "egc_gain_sched.hpp"
#include "egc_pid.hpp"
#include <Arduino.h>          // for constrain()

namespace egc {

/* ───────────────────────────────────────────────────────────────
   Closed-loop controller:
   • Reads the flow sensor
   • Gain-schedules Ki (and α) with ExpParams
   • Runs a PID
   • Drives the pump in steps-per-second (SPS)
   ───────────────────────────────────────────────────────────── */
class Controller {
public:
    Controller(const EgcParams& p,
               IFlowSensor&     s,
               IPumpDriver&     d)
        : P(p), sensor(s), driver(d),
          sched(p.gain), pid()
    {
        pid.setKP(p.Kp);
        pid.setKD(p.Kd);                   // ← fixed field name
    }

    /* Call in the main loop.
       set_uL_per_min – desired flow
       Returns the SPS command actually sent to the pump. */
    float update(float set_uL_per_min)
    {
        /* 1. Sensor & error */
        float raw   = sensor.read_uL_per_min();
        float flow  = P.scale.a * raw + P.scale.b;
        float err   = set_uL_per_min - flow;

        /* 2. Gain scheduling */
        float e_dyn, Ki, alpha_dyn;
        sched.update(err, e_dyn, Ki, alpha_dyn);

        /* 3. Δt for PID */
        unsigned long now = millis();
        float dt = (now - last_ms) * 1e-3f;   // ms → s
        last_ms  = now;

        /* 4. PID output ∈ [0,1] */
        float u_frac = pid.update(err, e_dyn, Ki, dt);
        u_frac = constrain(u_frac, 0.0f, 1.0f);

        /* 5. Scale to SPS & drive pump */
        float sps = u_frac * P.sps_max;
        driver.setTargetSPS(sps);
        return sps;
    }

    void reset()
    {
        sched.reset();
        pid.reset();
        last_ms = millis();
    }

private:
    const EgcParams&  P;
    IFlowSensor&      sensor;
    IPumpDriver&      driver;
    GainScheduler     sched;
    PID               pid;
    unsigned long     last_ms{millis()};
};

} // namespace egc
