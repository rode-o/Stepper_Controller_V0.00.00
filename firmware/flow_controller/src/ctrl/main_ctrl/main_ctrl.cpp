/*─────────────────────────────────────────────────────────────*/
/*  main_ctrl.cpp – top-level 100 Hz control loop              */
/*─────────────────────────────────────────────────────────────*/
#include <memory>
#include "main_ctrl.hpp"

#include "../_ctrl.hpp"              // ControlMode enum, controllers
#include "../../devices/_devices.hpp"
#include "../../include/_include.hpp"

using namespace State;

/* ── singletons ───────────────────────────────────────────── */
static std::unique_ptr<IController> gCtrl;
static ButtonsTwo    gButtons;
static Sh1107Display gDisplay;

/* ── helpers ──────────────────────────────────────────────── */
static void rebuild(ControlMode m)
{
    /* concrete controller selection */
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
    setCtrlMode(newMode);     // update SystemState
    rebuild(newMode);         // swap controller
    gDisplay.setPage(0);      // land on MODE page
}

void MainCtrl::toggleMode()
{
    ControlMode next = (read().ctrlMode == ControlMode::CLOSED)
                     ? ControlMode::OPEN
                     : ControlMode::CLOSED;
    onModeChanged(next);
}

/* ── setup – call once from Arduino setup() ───────────────── */
void MainCtrl::setup()
{
    /* persistent state & serial */
    loadPersistent();
    Serial.begin(115200);
    while (!Serial && millis() < 2000) {/* wait for USB */}

    /* ===== force safe defaults ===== */
    State::setPumpEnabled(false);
    State::setSystemOn(false);
    PumpDrv::stop();                       // ensure driver is idle

    /* I²C, buttons, display */
    Wire.begin(); Wire.setClock(400'000);
    gButtons.begin();
    gDisplay.begin();

    /* sensors & drivers */
    if (!startFlowMeasurement())
        Serial.println(F("[MC] Flow sensor init FAILED"));
    PumpDrv::initPump();                   // driver pins
    PumpDrv::setTop(0);                    // idle

    /* first controller instantiation */
    rebuild(read().ctrlMode);

    /* timestamp base for volume tracker */
    g_state.currentTimeMs = millis();
}

/* ── 100 Hz tick – call from Arduino loop() ──────────────── */
void MainCtrl::loop100Hz()
{
    static uint32_t last = millis();
    uint32_t now = millis();
    if (now - last < 10) return;           // 100 Hz guard
    uint32_t dt = now - last; last = now;
    g_state.currentTimeMs = now;

    /* ---------- UI inputs ---------- */
    gButtons.poll();
    if (gButtons.pageChanged())
        gDisplay.setPage(gButtons.currentPage());    // exact sync

    /* ---------- sensor ---------- */
    float filt = readAndFilterFlow();                // updates State

    /* ---------- totals ---------- */
    updateVolume(filt, dt);

    /* ---------- control ---------- */
    gCtrl->loop();

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
