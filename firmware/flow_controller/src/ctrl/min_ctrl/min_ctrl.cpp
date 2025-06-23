#include "min_ctrl.hpp"
#include "../../min_main.hpp"                 // umbrella: devices, core, utils…
#include <PID_v1.h>

#ifdef ENABLE_MIN_CTRL

/* ── singletons ─────────────────────────────────────────── */
static ButtonsTwo    gButtons;
static Sh1107Display gDisplay;
static VolumeTracker gVolume(0.97f);          // density ρ = 0.97 g mL⁻¹

/* ── tube / pump constants ─────────────────────────────── */
constexpr float UL_PER_REV  = 42.0f;          // µL delivered per shaft rev
constexpr float START_RPM   = 60.0f;
constexpr float START_uL    = START_RPM * UL_PER_REV;   // 60 rpm → 2 520 µL/min

/* ── global state (defined in system_state.cpp) ────────── */
extern volatile SystemState g_state;

/* ── PID objects (pv & sp in µL/min, out in rpm) ───────── */
double pv  = 0.0;                             // process value  [µL / min]
double sp  = START_uL;                        // set-point      [µL / min]
double out = START_RPM;                       // controller out [RPM]

/*    Gains are still “RPM per µL/min”.  Tune later if needed.    */
PID flowPid(&pv, &out, &sp, 5.0, 0.5, 0.0, DIRECT);

/* ── volume integration timing ─────────────────────────── */
static unsigned long prevVolMs = 0;

/* ── SETUP ─────────────────────────────────────────────── */
void ctrlSetup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 2000) {}

    Wire.begin();
    gButtons.begin();
    gDisplay.begin();
    Wire.setClock(400000);

    if (!startFlowMeasurement())
        Serial.println(F("[MIN_CTRL] Flow-sensor init FAILED"));

    PumpDrv::initPump();
    PumpDrv::setTargetRPM(0);                 // user enables later

    g_state.setpoint   = START_uL;            // store in µL / min
    g_state.volume_uL  = 0;
    g_state.mass_g     = 0;

    flowPid.SetOutputLimits(0, 200);          // RPM clamp
    flowPid.SetSampleTime(100);               // 100 ms loop
    flowPid.SetMode(AUTOMATIC);

    prevVolMs = millis();
}

/* ── LOOP ──────────────────────────────────────────────── */
void ctrlLoop()
{
    g_state.currentTimeMs = millis();

    /* 1. buttons -------------------------------------------------- */
    gButtons.poll();
    if (gButtons.pageChanged()) gDisplay.advancePage();

    /* 2. sensor read --------------------------------------------- */
    float flow_mL = readFlow();               // mL / min (already calibrated)
    pv            = flow_mL * 1000.0f;        // convert to µL / min
    g_state.flow  = flow_mL;                  // keep mL/min for UI

    /* ---- volume accumulator ------------------------------------ */
    unsigned long now = g_state.currentTimeMs;
    gVolume.update(pv, now - prevVolMs);      // pv is already µL / min
    prevVolMs = now;

    g_state.volume_uL = gVolume.volume_uL();
    g_state.mass_g    = gVolume.mass_g();

    /* 3. PID & pump gating --------------------------------------- */
    sp = g_state.setpoint;                    // µL / min from ButtonsTwo
    if (g_state.pumpEnabled) {
        flowPid.Compute();                    // out updated (RPM)
        g_state.rpmCmd = out;
    } else {
        out            = 0;
        g_state.rpmCmd = 0;
    }

    PumpDrv::setTargetRPM(out);               // driver still wants RPM
    PumpDrv::pumpService();
    g_state.spsCmd = PumpDrv::currentSPS();

    /* 4. JSON report every 250 ms -------------------------------- */
    static uint32_t last = 0;
    if (g_state.currentTimeMs - last >= 250) {
        last = g_state.currentTimeMs;
        SerialRpt::emitJSON(g_state);
    }

    /* 5. OLED refresh -------------------------------------------- */
    gDisplay.show(State::read());
}

#endif /* ENABLE_MIN_CTRL */
