/*─────────────────────────────────────────────────────────────*/
/*  main_ctrl.cpp – top-level 100 Hz control loop              */
/*─────────────────────────────────────────────────────────────*/
#include <memory>
#include "main_ctrl.hpp"

#include "../_ctrl.hpp"              // ControlMode enum, controllers
#include "../../devices/_devices.hpp"
#include "../../include/_include.hpp"
#include "../../devices/rgb/rgb.hpp"
#include "../../serial/_serial.hpp"  // SerialRpt, SerialCmd

using namespace State;

/* ── singletons ───────────────────────────────────────────── */
static std::unique_ptr<IController> gCtrl;
static ButtonsTwo    gButtons;
static Sh1107Display gDisplay;

/* ── helpers ──────────────────────────────────────────────── */
static void rebuild(ControlMode m)
{
    if (m == ControlMode::OPEN)
        gCtrl = std::make_unique<OpenLoopCtrl>();
    else
        gCtrl = std::make_unique<ClosedLoopCtrl>();
    gCtrl->setup();
}

/*------------------------------------------------------------*/
/*  Mode-change interface                                     */
/*------------------------------------------------------------*/
void MainCtrl::onModeChanged(ControlMode newMode)
{
    setCtrlMode(newMode);
    rebuild(newMode);                // swap controller impl
}

void MainCtrl::toggleMode()
{
    ControlMode next = (read().ctrlMode == ControlMode::CLOSED)
                     ? ControlMode::OPEN
                     : ControlMode::CLOSED;
    onModeChanged(next);
}

/* ── setup (call once from Arduino setup()) ───────────────── */
void MainCtrl::setup()
{
    loadPersistent();
    Serial.begin(115200);
    while (!Serial && millis() < 2000) {/* wait for USB */}

    SerialCmd::begin();              // ▲ initialise serial-command layer

    /* safe defaults */
    State::setPumpEnabled(false);
    State::setSystemOn(false);
    PumpDrv::stop();

    /* I²C, buttons, display */
    Wire.begin();
    Wire.setClock(400'000);
    gButtons.begin();
    gDisplay.begin();
    RGB::begin();                    // RGB LED

    /* sensors & drivers */
    if (!startFlowMeasurement())
        Serial.println(F("[MC] Flow sensor init FAILED"));
    PumpDrv::initPump();
    PumpDrv::setTop(0);

    rebuild(read().ctrlMode);        // first controller
    g_state.currentTimeMs = millis();
}

/* ── 100 Hz tick (call from Arduino loop()) ───────────────── */
void MainCtrl::loop100Hz()
{
    static uint32_t last = millis();
    uint32_t now = millis();
    if (now - last < 10) return;     // 100 Hz guard
    uint32_t dt = now - last;
    last = now;
    g_state.currentTimeMs = now;

    SerialCmd::poll();               // ▲ parse host commands

    /* ---------- UI inputs ---------- */
    gButtons.poll();
    if (gButtons.pageChanged()) {
        uint8_t next = gButtons.currentPage();
        if (next != gDisplay.currentPage()) {
            Serial.print(F("[DBG] loop -> animatedTo page "));
            Serial.println(next);
            gDisplay.animatedTo(next, State::read());
        }
    }

    /* ---------- sensor ---------- */
    float filt = readAndFilterFlow();   // updates State

    /* ---------- totals ---------- */
    updateVolume(filt, dt);

    /* ---------- control ---------- */
    gCtrl->loop();

    /* ---------- LED animation ---------- */
    if (State::isPumpEnabled()) {
        if (read().ctrlMode == ControlMode::CLOSED)
            RGB::loopCool();
        else
            RGB::loopWarm();
    }

    /* ---------- telemetry ---------- */
    SerialRpt::emitJSON(read());

    /* ---------- persistence ---------- */
    static uint32_t lastFlush = 0;
    if (now - lastFlush >= 5000) {
        commitPersistent();
        lastFlush = now;
    }

    /* ---------- draw ---------- */
    gDisplay.show(read());
}
