#include "../../include/_include.hpp"            // (ENABLE flags, VPR_DEFAULT…)
#include "utils_ctrl.hpp"
#include "../../main.hpp"
#include "../../devices/_devices.hpp"            // readFlow(), PumpDrv…
#include <Wire.h>

/*──────────────────────────────────────────────*/
/*  2-section DF-II bi-quad low-pass for flow    */
/*──────────────────────────────────────────────*/
namespace {

class BiQuad {
public:
    BiQuad(float b0,float b1,float b2,float a1,float a2):
        b0_(b0), b1_(b1), b2_(b2), a1_(a1), a2_(a2) {}
    float operator()(float x)
    {
        float v = x - a1_*z1_ - a2_*z2_;
        float y = b0_*v + b1_*z1_ + b2_*z2_;
        z2_ = z1_; z1_ = v;
        return y;
    }
private:
    float b0_, b1_, b2_, a1_, a2_, z1_ = 0, z2_ = 0;
};

/* 0.5 Hz corner (@ 1 kHz exec rate) */
constexpr float G0 = 0.0000613151978f,
                G1 = 0.0000608014289f;

static BiQuad bi0(1*G0, 2*G0, 1*G0, -1.98780471f, 0.98804997f);
static BiQuad bi1(1*G1, 2*G1, 1*G1, -1.97114861f, 0.97139181f);

/* ─ cumulative volume (authoritative) ─ */
static double accVol_uL = 0.0;                 // double avoids rollover
constexpr float DENSITY_G_PER_ML = 0.97f;

} // namespace



/*────────────────────────  PWM helpers  ───────────────────────*/
/* Convert requested flow-rate (µL min⁻¹) → PWM TOP value.       */
uint16_t rateToTop(double uLmin)
{
    constexpr double SYSCLK = 125'000'000.0;    // RP2040 sys clk (Hz)
    constexpr double PPR    = STEPS_PER_REV;    // motor steps per rev

    /* Use calibrated VPR (µL/rev); fall back if not set yet      */
    double vpr = static_cast<double>(State::read().vpr_uL_rev);
    if (vpr < 1.0e-3) vpr = static_cast<double>(VPR_DEFAULT);

    double rpm = uLmin / vpr;                   // µL min⁻¹ → rev min⁻¹
    double fq  = (rpm / 60.0) * PPR;            // rev min⁻¹ → steps s⁻¹

    /* RP2040 PWM: f_step = SYSCLK / [CLKDIV × (TOP + 1)]         */
    double top = SYSCLK / (PWM_CLKDIV * fq) - 1.0;

    if (top < 1)      top = 1;
    if (top > 65535)  top = 65535;
    return static_cast<uint16_t>(top);
}

float topToSps(uint16_t top)
{
    constexpr double SYSCLK = 125'000'000.0;
    return SYSCLK / (PWM_CLKDIV * (top + 1));
}

uint16_t spsToTop(float sps)
{
    if (sps <= 0.0f) return 0;                  // guard / stop
    constexpr double SYSCLK = 125'000'000.0;
    double top = SYSCLK / (PWM_CLKDIV * sps) - 1.0;

    if (top < 1)      top = 1;
    if (top > 65535)  top = 65535;
    return static_cast<uint16_t>(top);
}

/*────────────────────── sensor + totals helpers ─────────────────────*/
float readAndFilterFlow()
{
    float raw = readFlow();                     // sensor driver
    State::setRawFlow(raw);

    float filt = bi1(bi0(raw));                 // 2-section LPF
    State::setFiltFlow(filt);
    return filt;
}

void updateVolume(float flow_uLmin, uint32_t dtMs)
{
    /* ΔV = (Q / 60 000) · Δt   [µL] */
    double dV = static_cast<double>(flow_uLmin) *
                static_cast<double>(dtMs) / 60000.0;

    g_state.volume_uL += static_cast<float>(dV);
    g_state.mass_g     = g_state.volume_uL * DENSITY_G_PER_ML / 1000.0f;
}
