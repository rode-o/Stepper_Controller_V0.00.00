/**********************************************************************
 *  open_ctrl.cpp  – “open-loop” calibration pass
 *
 *  • Runs the pump at full speed (max SPS) for CAL_SETTLE_MS + CAL_WINDOW_MS
 *  • Calls egc::runCalibration() once (dual-plateau recipe)
 *  • Saves fitted EgcParams to EEPROM
 *  • Raises gCalibRunning so the OLED progress bar shows
 *********************************************************************/

#include "open_ctrl.hpp"
#include "../exp_ctrl/egc_calibration_config.hpp"     // CAL_SETTLE_MS, CAL_WINDOW_MS
#include "../../ctrl/_ctrl.hpp"                       // State helpers / SystemState
#include "../../devices/_devices.hpp"                 // PumpDrv
#include <EEPROM.h>

/* ---------- externs supplied by min_ctrl.cpp ---------- */
extern egc::IFlowSensor&   gSensor;
extern egc::IPumpDriver&   gPump;
extern volatile bool       gCalibRunning;
extern volatile uint32_t   gCalibStart;
extern volatile SystemState g_state;

/* ---------- local EEPROM blob definition --------------- */
struct CalBlob { uint32_t magic = 0xC0DEC0DE; egc::EgcParams p; };

/* ---------- module-local flag -------------------------- */
static bool done = false;

/* ---------- API called by mode dispatcher -------------- */
void openSetup()
{
    done = false;
    PumpDrv::initPump();
    PumpDrv::setTargetSPS(0);                      // ★ MOD: zero SPS (was RPM)
}

void openLoop()
{
    if (done) {                 // one-shot guard
        return;                 // caller re-enters closed-loop mode
    }

    /* —— flag UI —— */
    gCalibRunning       = true;
    gCalibStart         = millis();
    g_state.calibrating = true;

    /* —— run calibration —— */
    egc::EgcParams fresh{};

    /* positional initialiser follows struct order in egc_types.hpp */
    egc::CalConfig cfg{
        /* 1-2  target flow & open-loop speed */
        .f_nom_uL_min = g_state.setpoint,          // desired flow at steady-state
        .sps_max      = 2000.0f,                  // ★ MOD: absolute SPS to run pump

        /* 3-5  timing */
        .settle_ms    = CAL_SETTLE_MS,
        .window_ms    = CAL_WINDOW_MS,
        .sample_ms    = 100,                      // 100-ms polling

        /* 6-9  gain-shape knobs */
        .Ki_min       = 0.00f,
        .Ki_max       = 0.40f,
        .alpha_static = 0.20f,
        .knee_frac    = 0.50f,

        /* 10   stability gate */
        .stab_pct     = 2.0f
    };

    if (egc::runCalibration(cfg, gSensor, gPump, fresh)) {
        CalBlob blob{0xC0DEC0DE, fresh};
        EEPROM.put(0, blob);
    #if defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
        EEPROM.commit();
    #endif
    }

    /* —— clean-up —— */
    done                = true;
    gCalibRunning       = false;
    g_state.calibrating = false;
}
