#pragma once
/*  volume_tracker.hpp ─ cumulative volume / mass integrator
 *  ---------------------------------------------------------
 *  • update()  — integrate flow (µL / min) over Δt (ms)
 *  • reset()   — clear running totals
 *  • getters   — volume_uL(), mass_g()
 */

class VolumeTracker {
public:
    explicit VolumeTracker(float density_g_per_mL = 1.0f);

    void  update(float flow_uL_per_min, unsigned long dt_ms);
    void  reset();

    float volume_uL() const;
    float mass_g()   const;

private:
    float  _density;        // g / mL
    double _vol_uL{};       // use double to avoid rollover
};
