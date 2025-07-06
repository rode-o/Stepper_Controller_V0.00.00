/*─────────────────────────────────────────────────────────────*/
/*  buttons_two.cpp – three-page carousel edition              */
/*─────────────────────────────────────────────────────────────*/
#include "buttons_two.hpp"
#include "../../../main.hpp"                 // PIN defs, State helpers

#ifdef ENABLE_BUTTONS_TWO

/* quick RGB flasher */
namespace RGB {
inline void flash(LEDColour a, LEDColour b, uint16_t d = 150)
{
    setColour(a); delay(d);
    setColour(b); delay(d);
    setColour(State::isPumpEnabled() ? LED_GREEN : LED_RED);
}
}

/* ---- LED helper ---- */
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

    mFlowVal     = static_cast<int32_t>(State::read().setFlow_uLmin);
    mRpmVal      = static_cast<int16_t>(State::read().setRpm);
    mCalIdx      = static_cast<int16_t>(State::read().calScalar);
    mPumpEnabled = State::isPumpEnabled();
    updateLED();
    return true;
}

/* ───── pageChanged() – one-shot getter ───── */
bool ButtonsTwo::pageChanged()
{
    bool e = mPageEdge;
    mPageEdge = false;          // consume edge
    return e;
}

/* ───── poll() ───── */
void ButtonsTwo::poll()
{
    /* clear edge at start; will be re-set only if carousel flips */
    mPageEdge = false;

    bool upLow  = digitalRead(PIN_BTN_UP) == LOW;
    bool dnLow  = digitalRead(PIN_BTN_DN) == LOW;
    uint8_t mask = (upLow ? 1 : 0) | (dnLow ? 2 : 0);
    uint32_t now = millis();

    /* ===== dual-press logic ===== */
    if (mask == 3) {                               // both held
        if (!mDualActive) {
            mDualActive  = true;
            mDualStart   = now;
            mPumpLatched = false;
        }
        uint32_t held = now - mDualStart;

        /* 5 s → pump toggle */
        if (held >= PUMP_HOLD_MS && !mPumpLatched) {
            mPumpLatched = true;
            mPumpEnabled = !mPumpEnabled;
            State::setPumpEnabled(mPumpEnabled);
            updateLED();
            Serial.println(mPumpEnabled ? F("[BTN] Pump ENABLED")
                                        : F("[BTN] Pump DISABLED"));
        }
    } else {                                       // released
        if (mDualActive) {
            uint32_t held = now - mDualStart;
            if (!mPumpLatched && held >= PAGE_HOLD_MS && held < PUMP_HOLD_MS)
            {   /* three-page carousel */
                switch (mPage) {
                    case Page::SETPOINT:  mPage = Page::MEASURE;   break;
                    case Page::MEASURE:   mPage = Page::CALSCALAR; break;
                    default:              mPage = Page::SETPOINT;
                }
                mPageEdge = true;
                Serial.print(F("[DBG] Carousel -> "));
                Serial.println(static_cast<int>(mPage));
            }
            mDualActive = false;
        }
    }

    /* ===== debounced single-taps ===== */
    static uint32_t lastDeb = 0;
    if (mask != mLastMask && now - lastDeb > DEBOUNCE_MS) {

        bool upRel = (mLastMask == 1 && mask == 0);
        bool dnRel = (mLastMask == 2 && mask == 0);

        /* ---- MEASURE page : toggle control-mode ---- */
        if (mPage == Page::MEASURE && (upRel || dnRel)) {
            auto cur  = State::read().ctrlMode;
            auto next = (cur == ControlMode::CLOSED)
                      ? ControlMode::OPEN
                      : ControlMode::CLOSED;

            State::setCtrlMode(next);
            MainCtrl::onModeChanged(next);
            RGB::flash(LED_BLUE, LED_GREEN);
        }

        /* ---- SETPOINT page edits ---- */
        if (mPage == Page::SETPOINT) {
            if (State::read().ctrlMode == ControlMode::CLOSED) {
                if (upRel && mFlowVal <= 64000 - 25) mFlowVal += 25;
                if (dnRel && mFlowVal >= 25)         mFlowVal -= 25;
                State::setFlow(static_cast<float>(mFlowVal));
                announce("Flow", mFlowVal);
            } else {                                // OPEN
                if (upRel && mRpmVal <= 32767 - 1)  mRpmVal += 1;
                if (dnRel && mRpmVal >= 1)          mRpmVal -= 1;
                State::setRpm(static_cast<float>(mRpmVal));
                announce("RPM", mRpmVal);
            }
        }

        /* ---- CAL page edits ---- */
        if (mPage == Page::CALSCALAR) {
            if (upRel && mCalIdx <  50) ++mCalIdx;
            if (dnRel && mCalIdx > -50) --mCalIdx;
            State::setCalScalar(static_cast<float>(mCalIdx));
            announce("Cal%", mCalIdx);
        }

        lastDeb = now;
    }
    mLastMask = mask;

    /* ===== Up long-hold → systemOn toggle ===== */
    static uint32_t upHold = 0;
    if (upLow && !dnLow) {
        if (!upHold) upHold = now;
        else if (now - upHold >= 1000) {
            bool sys = !State::isSystemOn();
            State::setSystemOn(sys);
            Serial.println(sys ? F("[BTN] System ON")
                               : F("[BTN] System OFF"));
            upHold = 0;
        }
    } else upHold = 0;
}

/* ---- helper ---- */
void ButtonsTwo::announce(const char* tag, float v) const
{
    Serial.print(F("[BTN] "));
    Serial.print(tag);
    Serial.print(F(": "));
    Serial.println(v, 0);
}

#endif /* ENABLE_BUTTONS_TWO */
