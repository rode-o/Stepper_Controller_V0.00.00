#include "egc_calibrator.hpp"
#include <Arduino.h>          // for Serial prints

namespace egc {

bool runCalibration(const CalConfig& cfg,
                    IFlowSensor& sensor,
                    IPumpDriver& pump,
                    EgcParams&   out)
{
    /* ───── Sanity checks up-front ───── */
    if (cfg.Ki_max <= cfg.Ki_min) {
        Serial.println(F("[CAL] E1: Ki_max ≤ Ki_min"));
        return false;
    }
    if (cfg.knee_frac <= 0.0f || cfg.knee_frac >= 1.0f) {
        Serial.println(F("[CAL] E2: knee_frac out of range"));
        return false;
    }

    /* ───── A. open-loop pulse in SPS ───── */
    Serial.print(F("[DBG] sps_max="));
    Serial.println(cfg.sps_max, 1);                 // ★ MOD: report SPS

    pump.setTargetSPS(cfg.sps_max);                 // ★ MOD: drive pump by SPS
    delay(cfg.settle_ms);

    const uint32_t t0 = millis();
    float sum  = 0.0f, sum2 = 0.0f;
    uint16_t n = 0;

    while (millis() - t0 < cfg.window_ms) {
        float f = sensor.read_uL_per_min();
        sum  += f;
        sum2 += f * f;
        ++n;
        delay(cfg.sample_ms);                       // variable cadence
    }
    pump.stop();

    if (n == 0) {
        Serial.println(F("[CAL] E3: sensor produced no samples"));
        return false;
    }
    float mean = sum / n;
    if (mean < 1e-3f) {
        Serial.println(F("[CAL] E4: mean flow ≈ 0"));
        return false;
    }

    float var = sum2 / n - mean * mean;
    if (var < 0.0f) var = 0.0f;                     // numerical safety
    float cv  = 100.0f * sqrtf(var) / mean;         // coefficient of variation

    if (cv > cfg.stab_pct) {
        Serial.print(F("[CAL] E5: flow unstable, CV="));
        Serial.println(cv, 1);
        return false;
    }

    /* ───── B. solve amplitude & Ki curve ─────
       err_upper = steady-state error at full drive
       t_ref     = knee_frac · |err_upper|
       B chosen so curve spans Ki_min→Ki_max over ±2·t_ref               */
    float err_upper = cfg.f_nom_uL_min - mean;
    float t_ref     = cfg.knee_frac * fabsf(err_upper);
    if (t_ref < 1.0f) t_ref = 1.0f;                 // floor at 1 µL/min

    const float B = (cfg.Ki_max - cfg.Ki_min) / (4.0f * t_ref * t_ref);

    out.scale               = {};                   // unit scale; gravimetric later
    out.gain.A              = cfg.Ki_min;
    out.gain.K              = cfg.Ki_max;
    out.gain.B              = B;
    out.gain.c              = 0.0f;
    out.gain.t_ref          = t_ref;
    out.gain.alpha_static   = cfg.alpha_static;

    /* optional “soft slope” branch near zero error */
    out.gain.A2 = 0.05f;
    out.gain.B2 = B * 0.5f;
    out.gain.K2 = 0.95f;
    out.gain.c2 = 0.0f;

    Serial.println(F("[CAL] SUCCESS"));
    return true;
}

} // namespace egc
