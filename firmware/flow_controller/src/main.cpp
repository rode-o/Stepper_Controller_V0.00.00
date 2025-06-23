#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include "main.hpp"

/* ───────── Device-level includes ───────── */
#include "devices/_devices.hpp"

#ifdef ENABLE_FLOW_SENSOR

/* ───────── Globals ─────────────────────── */
static SystemState g_systemState;

static ButtonsTwo      gButtons;
static Sh1107Display   gDisplay;

static unsigned long startTime          = 0;
static bool           timeReporting     = false;
static bool           previousSystemOn  = false;
static bool           pageEdge          = false;

/* ───────── Local helpers ───────────────── */
void initButtons()   { gButtons.begin(); }
void updateButtons() {
    gButtons.poll();
    pageEdge = gButtons.modeToggle();
}
bool isSystemOn()            { return gButtons.systemOn(); }
bool wasModeTogglePressed()  { return pageEdge; }

void initPump()      { /* drv8825.begin();  (add real code) */ }
void stopPump()      { /* drv8825.stop();   (add real code) */ }

void initDisplay()   { gDisplay.begin(); }

void showStatus(const SystemState& s, bool edge)
{
    if (edge) gDisplay.advancePage();
    gDisplay.show(s);
}

/* ───────── Application entry points ────── */
void mainSetup()
{
    Serial.begin(115200);
    Wire.begin();
    EEPROM.begin(512);

    initButtons();
    initPump();
    initDisplay();

    initExpController(g_systemState);
    initConstantVoltageControl();

    if (!startFlowMeasurement())
        Serial.println("[MAIN] startFlowMeasurement() failed!");

    g_systemState.controlMode = CONTROL_MODE_EXP;
    startTime = millis();

    Serial.println("[MAIN] Setup complete → enter loop");
}

void mainLoop()
{
    /* optional serial toggle for loop timing */
    if (Serial.available()) {
        char c = Serial.read();
        if (c == 'T' || c == 't') {
            timeReporting = !timeReporting;
            Serial.print("[MAIN] Timing report ");
            Serial.println(timeReporting ? "ON" : "OFF");
        }
    }

    /* ─── Buttons & mode toggle ──────────── */
    updateButtons();
    g_systemState.systemOn = isSystemOn();

    if (previousSystemOn != g_systemState.systemOn) {
        Serial.println(g_systemState.systemOn ?
            "[MAIN] System ON – re-init controller" :
            "[MAIN] System OFF – re-init controller");
        initExpController(g_systemState);
        previousSystemOn = g_systemState.systemOn;
    }

    if (wasModeTogglePressed()) {
        g_systemState.controlMode =
            (g_systemState.controlMode == CONTROL_MODE_EXP)
            ? CONTROL_MODE_CONST_VOLTAGE
            : CONTROL_MODE_EXP;
        Serial.print("[MAIN] Mode → ");
        Serial.println(g_systemState.controlMode == CONTROL_MODE_EXP ?
                       "EXP" : "CONST VOLT");
    }

    /* ─── Sensor acquisition ─────────────── */
    g_systemState.flow           = readFlow();
    g_systemState.setpoint       = getFlowSetpoint();
    g_systemState.errorPercent   = getErrorPercent();
    g_systemState.temperature    = getTempC();
    g_systemState.bubbleDetected = (getLastFlags() & 0x01);

    float desiredV = 0.0f, pidFrac = 0.0f;
    float pTerm = 0, iTerm = 0, dTerm = 0;

    /* ─── Control computation ─────────────── */
    if (g_systemState.systemOn) {
        if (g_systemState.controlMode == CONTROL_MODE_EXP) {
            updateExpController(g_systemState,
                                g_systemState.flow,
                                g_systemState.setpoint,
                                g_systemState.errorPercent,
                                g_systemState.systemOn,
                                desiredV, pidFrac,
                                g_systemState.bubbleDetected,
                                pTerm, iTerm, dTerm);
        } else {
            updateConstantVoltageControl(g_systemState.systemOn, desiredV);
        }
    } else {
        stopPump();
    }

    /* store outputs */
    g_systemState.desiredVoltage = desiredV;
    g_systemState.pidOutput      = pidFrac;
    g_systemState.pTerm          = pTerm;
    g_systemState.iTerm          = iTerm;
    g_systemState.dTerm          = dTerm;

    /* ─── Display + JSON report ───────────── */
    showStatus(g_systemState, pageEdge);
    pageEdge = false;           // clear after use

    g_systemState.currentTimeMs = millis();
    reportAllStateJSON(g_systemState);

    /* ─── Auto-shutdown after timeout ─────── */
    if ((g_systemState.currentTimeMs - startTime) / 1000UL > SLF_RUN_DURATION)
    {
        stopFlowMeasurement();
        stopPump();
        Serial.println("[MAIN] Timer expired – halted");
        while (true) delay(100);
    }

    /* ─── Loop pacing & freq print ────────── */
    delay(MAIN_LOOP_DELAY_MS);

    static unsigned long loops = 0, lastMs = 0;
    ++loops;
    unsigned long now = millis();
    if (now - lastMs >= 2000) {
        float hz = 1000.0f * loops / (now - lastMs);
        Serial.print("[MAIN] ~"); Serial.print(hz,2); Serial.println(" Hz");
        loops = 0; lastMs = now;
    }
    if (timeReporting) Serial.println("[MAIN] loop done");
}


#endif // ENABLE_FLOW_SENSOR