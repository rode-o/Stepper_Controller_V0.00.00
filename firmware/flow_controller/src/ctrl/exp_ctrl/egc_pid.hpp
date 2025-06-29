#pragma once
#include <Arduino.h>

namespace egc {

class PID {
public:
    void setKD(float kd)  { Kd = kd; }
    void setKP(float kp)  { Kp = kp; }
    void reset()          { integ = 0; lastErr = 0; }

    float update(float err_raw,
                 float err_filtered,
                 float Ki_runtime,
                 float dt_s)
    {
        /* integral with clamped wind‑up */
        integ += Ki_runtime * err_filtered * dt_s;
        integ = constrain(integ, -imax, imax);

        float deriv = (err_raw - lastErr) / dt_s;
        lastErr = err_raw;

        float u = Kp*err_raw + integ + Kd*deriv;
        return constrain(u, 0.0f, 1.0f);
    }

private:
    float Kp{}, Kd{};
    float integ{}, lastErr{};
    const float imax{1.0f};     // anti‑wind‑up clamp
};

} // namespace egc
