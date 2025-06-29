#include "buttons_two.hpp"
#include "../../../include/_include.hpp"   // access g_state
#include "../../../min_main.hpp"           // PIN defs, RGB helpers

#ifdef ENABLE_BUTTONS_TWO

/* externs defined in min_ctrl.cpp */
extern volatile bool     gCalibRunning;
extern volatile uint32_t gCalibStart;

/* quick RGB flasher ----------------------------------------------------- */
namespace RGB {
inline void flash(LEDColour a, LEDColour b, uint16_t d = 150)
{
    setColour(a); delay(d);
    setColour(b); delay(d);
    setColour(State::isPumpEnabled() ? LED_GREEN : LED_RED);
}
}

/* internal helpers ------------------------------------------------------ */
void ButtonsTwo::updateLED()
{
    LEDColour c = mPumpEnabled ? LED_GREEN : LED_RED;
    State::setLEDColour(c);
    RGB::setColour(c);
}

/* ───── begin() ───── */
bool ButtonsTwo::begin()
{
    pinMode(PIN_BTN_UP, INPUT_PULLUP);
    pinMode(PIN_BTN_DN, INPUT_PULLUP);
    RGB::begin();

    mNumberVal   = static_cast<int32_t>(State::read().setpoint);
    mLetterIdx   = static_cast<int16_t>(State::read().calScalar);
    mPumpEnabled = State::isPumpEnabled();
    updateLED();
    return true;
}

/* defined in min_ctrl.cpp */
extern void startCalibrationAndStore();

/* ───── poll() ───── */
void ButtonsTwo::poll()
{
    /* lock-out during calibration */
    if (gCalibRunning) return;

    mPageEdge = false;

    bool upLow  = digitalRead(PIN_BTN_UP) == LOW;
    bool dnLow  = digitalRead(PIN_BTN_DN) == LOW;
    uint8_t mask = (upLow ? 1 : 0) | (dnLow ? 2 : 0);
    uint32_t now = millis();

    /* ===== dual-press logic ===== */
    if (mask == 3) {                          // both held
        if (!mDualActive) {
            mDualActive  = true;
            mDualStart   = now;
            mPumpLatched = false;
        }
        if (!mPumpLatched && (now - mDualStart) >= PUMP_HOLD_MS) {
            mPumpLatched = true;

            if (mMode == Mode::CALIB && !gCalibRunning) {
                /* ---- launch calibration ---- */
                gCalibRunning        = true;
                gCalibStart          = millis();
                extern volatile SystemState g_state;
                g_state.calibrating  = true;

                startCalibrationAndStore();   // blocks

                gCalibRunning        = false;
                g_state.calibrating  = false;

                /* -------- return to first page & refresh UI -------- */
                mMode     = Mode::SETPOINT;
                mPageEdge = true;             // ← forces redraw
                RGB::flash(LED_BLUE, LED_GREEN);

                /* ---------- reset button-state latches ------------- */
                mDualActive  = false;
                mPumpLatched = false;
                mLastMask    = 0;
            } else {                          // toggle pump
                mPumpEnabled = !mPumpEnabled;
                State::setPumpEnabled(mPumpEnabled);
                updateLED();
                Serial.print(F("[BTN] Pump "));
                Serial.println(mPumpEnabled ? F("ENABLED")
                                            : F("DISABLED"));
            }
        }
    } else {                                  // released
        if (mDualActive) {
            uint32_t held = now - mDualStart;
            if (!mPumpLatched &&
                held >= PAGE_HOLD_MS && held < PUMP_HOLD_MS)
            {   /* cycle pages */
                switch (mMode) {
                    case Mode::SETPOINT: mMode = Mode::MEASURE;     break;
                    case Mode::MEASURE:  mMode = Mode::CALSCALAR;   break;
                    case Mode::CALSCALAR:mMode = Mode::CALIB;       break;
                    default:             mMode = Mode::SETPOINT;
                }
                mPageEdge = true;
            }
            mDualActive = false;
        }
    }

    /* ===== single-button short presses ===== */
    static uint32_t lastDeb = 0;
    if (mask != mLastMask && now - lastDeb > DEBOUNCE_MS) {

        if (mLastMask == 1 && mask == 0) {          // UP release
            if (mMode == Mode::SETPOINT && mNumberVal <= 65000 - 10) {
                mNumberVal += 10; announce("Set", mNumberVal);
            }
            if (mMode == Mode::CALSCALAR && mLetterIdx < 50) {
                ++mLetterIdx; announce("Cal%", mLetterIdx);
            }
        }
        if (mLastMask == 2 && mask == 0) {          // DN release
            if (mMode == Mode::SETPOINT && mNumberVal >= 10) {
                mNumberVal -= 10; announce("Set", mNumberVal);
            }
            if (mMode == Mode::CALSCALAR && mLetterIdx > -50) {
                --mLetterIdx; announce("Cal%", mLetterIdx);
            }
        }
        lastDeb = now;
    }
    mLastMask = mask;

    /* ===== UP long-hold systemOn toggle ===== */
    static uint32_t upHold = 0;
    if (upLow && !dnLow) {
        if (!upHold) upHold = now;
        else if (now - upHold >= 1000) {
            bool sys = !State::isSystemOn();
            State::setSystemOn(sys);
            Serial.print(F("[BTN] System "));
            Serial.println(sys ? F("ON") : F("OFF"));
            upHold = 0;
        }
    } else upHold = 0;

    /* ===== push edits to global state ===== */
    State::setSetpoint (static_cast<float>(mNumberVal));
    State::setCalScalar(static_cast<float>(mLetterIdx));
}

/* helper --------------------------------------------------------------- */
void ButtonsTwo::announce(const char* tag, float v) const
{
    Serial.print(F("[BTN] "));
    Serial.print(tag);
    Serial.print(F(": "));
    if (tag[0] == 'S') Serial.println(v / 1000.0f, 2);  // “Set” value in mL/min
    else               Serial.println(v, 0);
}

#endif /* ENABLE_BUTTONS_TWO */
