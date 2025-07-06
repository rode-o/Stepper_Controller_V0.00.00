#include "volume_tracker.hpp"

VolumeTracker::VolumeTracker(float ρ) : _density(ρ) {}

void VolumeTracker::update(float q_uL_min, unsigned long dt_ms)
{
    /* ΔV = (Q / 60 000) · Δt   [µL] */
    _vol_uL += static_cast<double>(q_uL_min) *
               static_cast<double>(dt_ms) / 60000.0;
}

void  VolumeTracker::reset()     { _vol_uL = 0.0; }
float VolumeTracker::volume_uL() const { return static_cast<float>(_vol_uL); }
float VolumeTracker::mass_g()    const { return static_cast<float>(_vol_uL) *
                                                 _density / 1000.0f; }
