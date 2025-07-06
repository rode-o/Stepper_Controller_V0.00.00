#include "utils_ctrl.hpp"
#include "../../main.hpp"
#include <Wire.h>

/* ───────────────────────────── 2-section DF-II bi-quad LPF ───────────────────────────── */
namespace {
class BiQuad {
public:
    BiQuad(float b0,float b1,float b2,float a1,float a2):
        b0_(b0),b1_(b1),b2_(b2),a1_(a1),a2_(a2) {}
    float operator()(float x) {
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

/* cumulative totals */
static VolumeTracker gVol(0.97f);   // ρ = 0.97 g mL-¹
} // namespace

/* ───────────────────────────────────── PWM helpers ───────────────────────────────────── */
uint16_t rateToTop(double uLmin)
{
    constexpr double SYSCLK = 125'000'000.0;
    const     double vpr    = static_cast<double>(VPR);
    constexpr double PPR    = STEPS_PER_REV;

    double rpm = uLmin / vpr;               // µL min-¹ → rev min-¹
    double fq  = (rpm / 60.0) * PPR;        // rev min-¹ → steps s-¹

    /* RP2040 PWM: f_step = SYSCLK / (CLKDIV * (TOP + 1)) */
    double top = SYSCLK / (PWM_CLKDIV * fq) - 1.0;   // ← *2 removed

    if (top < 1)      top = 1;
    if (top > 65535)  top = 65535;
    return static_cast<uint16_t>(top);
}

float topToSps(uint16_t top)
{
    constexpr double SYSCLK = 125'000'000.0;
    return SYSCLK / (PWM_CLKDIV * (top + 1));        // ← *2 removed
}

uint16_t spsToTop(float sps)
{
    if (sps <= 0.0f) return 0;                       // guard / stop
    constexpr double SYSCLK = 125'000'000.0;
    double top = SYSCLK / (PWM_CLKDIV * sps) - 1.0;  // direct inverse

    if (top < 1)      top = 1;
    if (top > 65535)  top = 65535;
    return static_cast<uint16_t>(top);
}

/* ───────────────────────────── sensor + totals helpers ───────────────────────────── */
float readAndFilterFlow()
{
    float raw = readFlow();
    State::setRawFlow(raw);

    float filt = bi1(bi0(raw));
    State::setFiltFlow(filt);
    return filt;
}

void updateVolume(float flow_uLmin, uint32_t dtMs)
{
    gVol.update(flow_uLmin, dtMs);

    g_state.volume_uL = gVol.volume_uL();
    g_state.mass_g    = gVol.mass_g();
}
