/*
 *  min_ctrl.cpp — PID-based flow controller (RP2040 period-driven)
 *  ----------------------------------------------------------------
 *  All flow quantities are expressed in µL / min.
 */

#include "min_ctrl.hpp"
#include "../../min_main.hpp"
#include <Wire.h>
#include <PID_v1.h>

/* ─── stubs for deprecated auto-cal ─── */
volatile bool     gCalibRunning = false;
volatile uint32_t gCalibStart   = 0;
void startCalibrationAndStore() {}                   /* no-op */

#ifdef ENABLE_MIN_CTRL
extern volatile SystemState g_state;

/* UI helpers */
static ButtonsTwo    gButtons;
static Sh1107Display gDisplay;
static VolumeTracker gVolume(0.97f);                 // density ρ = 0.97 g/mL

static uint32_t prevVolMs = 0, lastJson = 0, lastFlush = 0;

/* ─── 2-section DF-II bi-quad LPF ─── */
class BiQuad {
public:
    BiQuad(float b0,float b1,float b2,float a1,float a2):
        b0_(b0),b1_(b1),b2_(b2),a1_(a1),a2_(a2) {}
    float operator()(float x){
        float v = x - a1_*z1_ - a2_*z2_;
        float y = b0_*v + b1_*z1_ + b2_*z2_;
        z2_ = z1_; z1_ = v; return y;
    }
private:
    float b0_, b1_, b2_, a1_, a2_, z1_ = 0, z2_ = 0;
};
/* ≈ 0.5 Hz corner if executed at 1 kHz */
constexpr float G0 = 0.0000613151978015f,
                G1 = 0.0000608014289594f;
static BiQuad biquad0(1*G0, 2*G0, 1*G0, -1.98780470979604f, 0.98804997058725f);
static BiQuad biquad1(1*G1, 2*G1, 1*G1, -1.97114860885104f, 0.97139181456688f);

/* ─── PID objects ─── */
static double gMeasuredRate = 0, gPidOutput = 0, gTargetRate = 0;
static PID    gPid(&gMeasuredRate, &gPidOutput, &gTargetRate,
                   1.0, 0.30, 0.0, DIRECT);          // Kp, Ki, Kd

/* Geometry helper (RPM telemetry) */
constexpr float STEPS_PER_REV = 200.0f * PumpDrv::MICROSTEP_DIV;

/* ───── µL/min → PWM TOP (wrap) ───── */
static inline uint16_t rateToTop(double uLmin)
{
    const double vpr        = static_cast<double>(VPR);
    constexpr double SYSCLK = 125'000'000.0;          // Hz
    constexpr double CLKDIV = 8.0;
    constexpr int    TPS    = 2;
    constexpr int    SPR    = 200;
    constexpr double TPR    = SPR * TPS;

    double rpm = uLmin / vpr;
    double fq  = (rpm / 60.0) * TPR;                  // steps / s
    double top = SYSCLK / (CLKDIV * 2.0 * fq) - 1.0;
    if (top < 1)     top = 1;
    if (top > 65535) top = 65535;
    return static_cast<uint16_t>(top);
}

/* ───── TOP → SPS (for truthful telemetry) ───── */
static inline float topToSps(uint16_t top)
{
    constexpr double SYSCLK = 125'000'000.0;
    constexpr double CLKDIV = 8.0;
    return SYSCLK / (CLKDIV * 2.0 * (top + 1));
}

/* ─── ctrlSetup ─── */
void ctrlSetup()
{
    State::loadPersistent();
    State::setPumpEnabled(false);

    Serial.begin(115200);
    while (!Serial && millis() < 2000) {/* wait for USB */}

    Wire.begin(); Wire.setClock(400'000);
    gButtons.begin(); gDisplay.begin();

    if (!startFlowMeasurement())
        Serial.println(F("[MIN_CTRL] Flow sensor init FAILED"));

    PumpDrv::initPump();                PumpDrv::setTop(0);

    if (g_state.setpoint == 0)          State::setSetpoint(500.0f);
    gTargetRate = g_state.setpoint;

    gPid.SetOutputLimits(50, 1500);     // low clamp raised from 0 → 50
    gPid.SetSampleTime(100);            // 100 ms → 10 Hz
    gPid.SetMode(AUTOMATIC);

    prevVolMs = millis();
}

/* 100 Hz scheduler */
constexpr uint16_t LOOP_DT_MS = 10;
static uint32_t    lastLoopMs = 0;

/* ─── ctrlLoop ─── */
void ctrlLoop()
{
    g_state.currentTimeMs = millis();
    uint32_t now = g_state.currentTimeMs;
    if (now - lastLoopMs < LOOP_DT_MS) return;
    lastLoopMs += LOOP_DT_MS;

    /* ---------- UI ---------- */
    gButtons.poll();
    if (gButtons.pageChanged()) gDisplay.advancePage();

    /* ---------- sensor ---------- */
    float rateRaw = readFlow();                 State::setRawFlow(rateRaw);
    gMeasuredRate = biquad1(biquad0(rateRaw));  State::setFiltFlow(gMeasuredRate);

    /* ---------- totals ---------- */
    gVolume.update(gMeasuredRate, now - prevVolMs);
    prevVolMs = now;
    g_state.volume_uL = gVolume.volume_uL();
    g_state.mass_g    = gVolume.mass_g();

    /* ---------- control ---------- */
    gTargetRate = g_state.setpoint;

    if (g_state.pumpEnabled) {
        gPid.Compute();                         // runs @ 10 Hz

        uint16_t top = rateToTop(gPidOutput);
        PumpDrv::setTop(top);
        State::setTop(top);                     // NEW → JSON shows "top"

        float spsCmd = topToSps(top);
        float rpmCmd = spsCmd * 60.0f / STEPS_PER_REV;

        g_state.spsCmd = spsCmd;
        g_state.rpmCmd = rpmCmd;
    } else {
        PumpDrv::setTop(0);
        State::setTop(0);
        g_state.spsCmd = 0; g_state.rpmCmd = 0;
    }

    /* ---------- telemetry ---------- */
    if (now - lastJson >= 250) {
        lastJson = now;
        SerialRpt::emitJSON(g_state);
    }

    /* ---------- persistence ---------- */
    if (now - lastFlush >= 5000) {
        lastFlush = now;
        State::commitPersistent();
    }

    gDisplay.show(State::read());
}
#endif   /* ENABLE_MIN_CTRL */
