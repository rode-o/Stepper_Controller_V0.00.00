#include "min_ctrl.hpp"
#include "../../min_main.hpp"
#include <PID_v1.h>

#ifdef ENABLE_MIN_CTRL

static ButtonsTwo    gButtons;
static Sh1107Display gDisplay;
static VolumeTracker gVolume(0.97f);

constexpr float UL_PER_REV = 42.0f;
constexpr float START_RPM  = 60.0f;
constexpr float START_uL   = START_RPM * UL_PER_REV;

extern volatile SystemState g_state;

/* PID in µL/min -> RPM */
double pv  = 0, sp = START_uL, out = START_RPM;
PID flowPid(&pv, &out, &sp, 5.0, 0.5, 0.0, DIRECT);

static unsigned long prevVolMs = 0;

void ctrlSetup()
{
    State::loadPersistent(); 
    State::setPumpEnabled(false);                       // restore set-point & err%

    Serial.begin(115200);
    while (!Serial && millis() < 2000) {}

    Wire.begin();
    gButtons.begin();
    gDisplay.begin();
    Wire.setClock(400000);

    if (!startFlowMeasurement())
        Serial.println(F("[MIN_CTRL] Flow-sensor init FAILED"));

    PumpDrv::initPump();
    PumpDrv::setTargetRPM(0);

    if (g_state.setpoint == 0)                   // first boot fallback
        State::setSetpoint(START_uL);

    sp = g_state.setpoint;                       // sync PID

    flowPid.SetOutputLimits(0, 200);
    flowPid.SetSampleTime(100);
    flowPid.SetMode(AUTOMATIC);

    prevVolMs = millis();
}

void ctrlLoop()
{
    g_state.currentTimeMs = millis();

    gButtons.poll();
    if (gButtons.pageChanged()) gDisplay.advancePage();

    float flow_mL = readFlow();                  // mL/min
    pv            = flow_mL * 1000.0f;           // -> µL/min
    g_state.flow  = flow_mL;

    unsigned long now = g_state.currentTimeMs;
    gVolume.update(pv, now - prevVolMs);
    prevVolMs = now;

    g_state.volume_uL = gVolume.volume_uL();
    g_state.mass_g    = gVolume.mass_g();

    sp = g_state.setpoint;                       // live set-point
    if (g_state.pumpEnabled) {
        flowPid.Compute();
        g_state.rpmCmd = out;
    } else {
        out            = 0;
        g_state.rpmCmd = 0;
    }

    PumpDrv::setTargetRPM(out);
    PumpDrv::pumpService();
    g_state.spsCmd = PumpDrv::currentSPS();

    static uint32_t lastJson = 0;
    if (g_state.currentTimeMs - lastJson >= 250) {
        lastJson = g_state.currentTimeMs;
        SerialRpt::emitJSON(g_state);
    }

    static uint32_t lastFlush = 0;
    if (g_state.currentTimeMs - lastFlush >= 5000) {
        State::commitPersistent();
        lastFlush = g_state.currentTimeMs;
    }

    gDisplay.show(State::read());
}

#endif /* ENABLE_MIN_CTRL */
