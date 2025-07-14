/*─────────────────────────────────────────────────────────────*
 *  main_ctrl.cpp – top-level 100 Hz control loop              *
 *─────────────────────────────────────────────────────────────*/
#include "../../include/_include.hpp"        // (defines ENABLE_* flags, pins…)

#include <memory>
#include "main_ctrl.hpp"

#include "../_ctrl.hpp"                      // IController, ControlMode
#include "../../devices/_devices.hpp"
#include "../../devices/flow_sensors/SLF3S-0600F/SFL3S-0600F.hpp"
#include "../../serial/_serial.hpp"          // SerialRpt, SerialCmd
#include "../../ui/_ui.hpp"                  // UI::DisplayMgr, ButtonsTwo, …

using namespace State;

/* ─── per-board identity (pick UNIQUE 1-255) ───────────────────────── */
constexpr uint8_t DEVICE_ID = 3;             // ← change per board

/* ─── singletons ───────────────────────────────────────────────────── */
static std::unique_ptr<IController> gCtrl;
static ButtonsTwo         gButtons;
/* NOTE: not static → visible to ButtonsTwo for wizard events */
static UI::DisplayMgr     gDisplay;

/* ─── helpers ──────────────────────────────────────────────────────── */
static void rebuild(ControlMode m)
{
    if (m == ControlMode::OPEN)
        gCtrl = std::make_unique<OpenLoopCtrl>();   // now flow→RPM inside
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
    rebuild(newMode);                  // hot-swap controller
}

void MainCtrl::toggleMode()
{
    ControlMode next = (read().ctrlMode == ControlMode::CLOSED)
                     ? ControlMode::OPEN
                     : ControlMode::CLOSED;
    onModeChanged(next);
}

/* ── setup (call once from Arduino setup()) ───────────────────────── */
void MainCtrl::setup()
{
    loadPersistent();

    /* assign / override device ID EVERY boot */
    if (State::getDeviceId() != DEVICE_ID)
        State::setDeviceId(DEVICE_ID);

    Serial.begin(115200);
    while (!Serial && millis() < 2000) { /* wait for USB */ }

    SerialCmd::begin();                // ▲ serial-command layer

    /* safe defaults */
    State::setPumpEnabled(false);
    State::setSystemOn(false);
    PumpDrv::stop();

    /* I²C, buttons, display */
    Wire.begin();
    Wire.setClock(400'000);
    gButtons.begin();
    gDisplay.begin();
    RGB::begin();                      // onboard RGB LED

    /* sensors & drivers */
    if (!startFlowMeasurement())
        Serial.println(F("[MC] Flow sensor init FAILED"));
    PumpDrv::initPump();
    PumpDrv::setTop(0);

    rebuild(read().ctrlMode);          // first controller instance
    g_state.currentTimeMs = millis();
}

/* ── 100 Hz tick (call from Arduino loop()) ───────────────────────── */
void MainCtrl::loop100Hz()
{
    static uint32_t last = millis();
    uint32_t now = millis();
    if (now - last < 10) return;       // 100 Hz guard
    uint32_t dt = now - last;
    last = now;
    g_state.currentTimeMs = now;

    SerialCmd::poll();                 // ▲ parse host commands

    /* ---------- UI inputs ---------- */
    gButtons.poll();
    if (gButtons.pageChanged())
        gDisplay.setPage(gButtons.currentPage());

    /* ---------- sensor ---------- */
    float filt = readAndFilterFlow();  // updates State

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
    SerialRpt::emitJSON(read());       // now includes "id"

    /* ---------- persistence ---------- */
    static uint32_t lastFlush = 0;
    if (now - lastFlush >= 5000) {
        commitPersistent();
        lastFlush = now;
    }

    /* ---------- display ---------- */
    gDisplay.tick();                   // Cal-wizard timers + redraw
}
