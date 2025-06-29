#pragma once
#include <stdint.h>
#include <vector>

namespace egc {

/* ─── Hardware abstraction ──────────────────────────────── */
struct IFlowSensor {
    virtual float read_uL_per_min() = 0;
};

/* Pump interface now speaks SPS (steps-per-second) */
struct IPumpDriver {
    /* Primary API: absolute speed in SPS */
    virtual void  setTargetSPS(float sps) = 0;

    /* Convenience helpers */
    virtual void  stop()                  { setTargetSPS(0.0f); }

    /* ── Legacy wrapper (kept for older modules) ──────────
       Interprets the argument as a 0-to-1 fraction of the
       driver’s own MAX_SPS. Override in a concrete driver
       if you need different scaling, or remove once all
       callers are migrated.                                 */
    virtual void  setCommand(float dutyFrac) { setTargetSPS(dutyFrac); }

    virtual ~IPumpDriver() = default;
};

/* ─── Raw-to-true scale (e.g. ADC) ───────────────────────── */
struct ScaleAffine { float a = 1.0f, b = 0.0f; };

/* ─── Exponential gain-schedule parameters ───────────────── */
struct ExpParams {
    float A{}, B{}, K{}, c{}, t_ref{};
    float alpha_static{0.20f};

    /* secondary curve – soft slope near zero error */
    float A2{0.05f}, B2{0.001f}, K2{0.95f}, c2{0.0f};
};

/* ─── Bundle saved to flash (factory calibration) ────────── */
struct EgcParams {
    ScaleAffine scale;
    ExpParams   gain;
    float       Kp{0.0f}, Kd{0.0f};     // optional P/D
    float       sps_max{2000.0f};       // ★ NEW: max SPS used by controller
};

/* ─── Calibration-time knobs passed to runCalibration() ─── */
struct CalConfig {
    /* 1. Design-time target */
    float     f_nom_uL_min{};           // nominal set-point for fit

    /* 2. Open-loop pulse parameters (SPS, not voltage) */
    float     sps_max{2000.0f};         // ★ NEW: absolute speed for pulse
    uint32_t  settle_ms{2000};          // wait before sampling (ms)
    uint32_t  window_ms{800};           // sample window length (ms)
    uint16_t  sample_ms{50};            // sensor poll interval (ms)

    /* 3. Gain-shape knobs */
    float     Ki_min{0.0f}, Ki_max{0.40f};
    float     alpha_static{0.20f};
    float     knee_frac{0.5f};          // fraction of |err_upper| for knee

    /* 4. Quality gate */
    float     stab_pct{2.5f};           // max allowed CoV (%) for success
};

} // namespace egc
