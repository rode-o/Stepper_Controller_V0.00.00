/* buttons_two.cpp – dual-button UI handler
 * ─────────────────────────────────────────────────────────────
 *  • Short press            : ±10 µL / min
 *  • Dual-press 1–5 s       : cycle OLED pages
 *  • Dual-press ≥5 s        : toggle pump (RGB green ↔ red)
 *  • UP long-press ≥1 s     : toggle systemOn flag
 */
#include "buttons_two.hpp"

#ifdef ENABLE_BUTTONS_TWO

/* ───── private helper ─────────────────────────────────── */
void ButtonsTwo::updateLED()
{
    LEDColour c = mPumpEnabled ? LED_GREEN : LED_RED;
    State::setLEDColour(c);
    RGB::setColour(c);
}

/* ───── public: begin() ───────────────────────────────── */
bool ButtonsTwo::begin()
{
    pinMode(PIN_BTN_UP, INPUT_PULLUP);
    pinMode(PIN_BTN_DN, INPUT_PULLUP);

    RGB::begin();
    mPumpEnabled = State::isPumpEnabled();
    updateLED();
    return true;
}

/* ───── public: poll() ───────────────────────────────── */
void ButtonsTwo::poll()
{
    mPageEdge = false;

    /* -------- read buttons ---------- */
    bool upLow = (digitalRead(PIN_BTN_UP) == LOW);
    bool dnLow = (digitalRead(PIN_BTN_DN) == LOW);
    uint8_t mask = (upLow ? 1 : 0) | (dnLow ? 2 : 0);
    uint32_t now = millis();

    /* -------- dual-press block -------------------------- */
    if (mask == 3) {                               /* both held */
        if (!mDualActive) {                        /* new press */
            mDualActive  = true;
            mDualStart   = now;
            mPumpLatched = false;
        }
        /* still held – after 5 s toggle pump */
        if (!mPumpLatched && (now - mDualStart) >= PUMP_HOLD_MS) {
            mPumpLatched  = true;
            mPumpEnabled  = !mPumpEnabled;

            State::setPumpEnabled(mPumpEnabled);
            updateLED();

            Serial.print(F("[BTN] Pump "));
            Serial.println(mPumpEnabled ? F("ENABLED") : F("DISABLED"));
        }
    } else {                                       /* released or single-press */
        if (mDualActive) {                         /* was holding */
            uint32_t held = now - mDualStart;
            if (!mPumpLatched && held >= PAGE_HOLD_MS && held < PUMP_HOLD_MS) {
                /* 1 s ≤ held < 5 s → page change */
                if      (mMode == Mode::SETPOINT) mMode = Mode::MEASURE;
                else if (mMode == Mode::MEASURE)  mMode = Mode::ERROR;
                else                              mMode = Mode::SETPOINT;
                mPageEdge = true;
            }
            mDualActive = false;
        }
    }

    /* -------- single-press short actions --------------- */
    static uint32_t lastDebounce = 0;
    if (mask != mLastMask && now - lastDebounce > DEBOUNCE_MS) {

        /* UP released */
        if (mLastMask == 1 && mask == 0) {
            if (mMode == Mode::SETPOINT && mNumberVal <= 65000 - 10) {
                mNumberVal += 10;                  // +10 µL
                announce("Set", mNumberVal);
            }
            if (mMode == Mode::ERROR && mLetterIdx < 50) {
                ++mLetterIdx; announce("Err%", mLetterIdx);
            }
        }

        /* DN released */
        if (mLastMask == 2 && mask == 0) {
            if (mMode == Mode::SETPOINT && mNumberVal >= 10) {
                mNumberVal -= 10;                  // −10 µL
                announce("Set", mNumberVal);
            }
            if (mMode == Mode::ERROR && mLetterIdx > -50) {
                --mLetterIdx; announce("Err%", mLetterIdx);
            }
        }
        lastDebounce = now;
    }
    mLastMask = mask;

    /* -------- long-press UP → systemOn toggle ---------- */
    static uint32_t upHoldStart = 0;
    if (upLow && !dnLow) {
        if (!upHoldStart) upHoldStart = now;
        else if (now - upHoldStart >= 1000) {
            bool sys = !State::isSystemOn();
            State::setSystemOn(sys);

            Serial.print(F("[BTN] System "));
            Serial.println(sys ? F("ON") : F("OFF"));
            upHoldStart = 0;
        }
    } else {
        upHoldStart = 0;
    }

    /* -------- push edits to global state --------------- */
    SystemState& st = const_cast<SystemState&>(State::read());
    st.setpoint = static_cast<float>(mNumberVal);   // µL / min
    State::setErrorPercent(static_cast<float>(mLetterIdx));
}

/* ───── helper: announce() ───────────────────────────── */
void ButtonsTwo::announce(const char* tag, float v) const
{
    Serial.print(F("[BTN] "));
    Serial.print(tag);
    Serial.print(F(": "));
    if (tag[0] == 'S')                // set-point: show mL with 0.01 precision
        Serial.println(v / 1000.0f, 2);
    else
        Serial.println(v, 0);
}
#endif // ENABLE_BUTTONS_TWO
