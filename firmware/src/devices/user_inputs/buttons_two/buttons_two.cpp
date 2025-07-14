/*─────────────────────────────────────────────────────────────*/
/*  buttons_two.cpp – three-page carousel + Cal-wizard hooks   */
/*─────────────────────────────────────────────────────────────*/
#include "buttons_two.hpp"
#include "../../../main.hpp"                     // PIN defs, State helpers
#include "../../../ui/_ui.hpp"                   // Pages, DisplayMgr, …
#include "../../../ui/btn/btn.hpp"
#include "../../../ui/cal_wizard/cal_wizard.hpp" // gCalWizard.isEditingWeight()
#include <cstring>

using UI::Btn;           // resolve Btn::UP / DOWN / OK

#ifdef ENABLE_BUTTONS_TWO

/* ───────────────────────── RGB helper ──────────────────────*/
namespace RGB {
inline void flash(LEDColour a, LEDColour b, uint16_t d = 150)
{
    setColour(a); delay(d);
    setColour(b); delay(d);
    setColour(State::isPumpEnabled() ? LED_GREEN : LED_RED);
}
}

/* ───────────────────── LED helper ──────────────────────────*/
void ButtonsTwo::updateLED()
{
    LEDColour c = mPumpEnabled ? LED_GREEN : LED_RED;
    State::setLEDColour(c);
    RGB::setColour(c);
}

/* ───────────────────── begin() ─────────────────────────────*/
bool ButtonsTwo::begin()
{
    pinMode(PIN_BTN_UP, INPUT_PULLUP);
    pinMode(PIN_BTN_DN, INPUT_PULLUP);
    RGB::begin();

    mFlowVal     = static_cast<int32_t>(State::read().setFlow_uLmin);
    mPumpEnabled = State::isPumpEnabled();
    updateLED();
    return true;
}

/* ───────────── page-edge getter (one-shot) ─────────────────*/
bool ButtonsTwo::pageChanged()
{
    bool e = mPageEdge;
    mPageEdge = false;
    return e;
}

/* ───────────────────── poll() ──────────────────────────────*/
void ButtonsTwo::poll()
{
    /* clear edge each cycle; re-set only if carousel flips */
    mPageEdge = false;

    bool upLow = digitalRead(PIN_BTN_UP) == LOW;
    bool dnLow = digitalRead(PIN_BTN_DN) == LOW;
    uint8_t mask = (upLow ? 1 : 0) | (dnLow ? 2 : 0);
    uint32_t now = millis();

    /* ===== dual-press logic ===================================== */
    if (mask == 3) {                                 // both held
        if (!mDualActive) {
            mDualActive  = true;
            mDualStart   = now;
            mPumpLatched = false;
        }
        uint32_t held = now - mDualStart;

        /* very long hold → pump toggle or wizard OK --------------- */
        if (held >= PUMP_HOLD_MS && !mPumpLatched) {
            mPumpLatched = true;

            if (mPage == Page::CALSCALAR) {
                UI::Pages::handleCalButton(Btn::OK);     // long OK
            } else {
                mPumpEnabled = !mPumpEnabled;
                State::setPumpEnabled(mPumpEnabled);
                updateLED();
                Serial.println(mPumpEnabled ? F("[BTN] Pump ENABLED")
                                            : F("[BTN] Pump DISABLED"));
            }
        }
    }
    else {                                           // released
        if (mDualActive) {
            uint32_t held = now - mDualStart;

            /* ---- CASE 1: weight-editor caret step ---------------- */
            if (mPage == Page::CALSCALAR &&
                UI::gCalWizard.isEditingWeight() &&
                held < PAGE_HOLD_MS)
            {
                UI::Pages::handleCalButton(Btn::NEXT_DIGIT);
            }
            /* ---- CASE 2: normal carousel swipe ------------------ */
            else if (!mPumpLatched && held < PUMP_HOLD_MS)
            {
                switch (mPage) {
                    case Page::SETPOINT:  mPage = Page::MEASURE;   break;
                    case Page::MEASURE:   mPage = Page::CALSCALAR; break;
                    default:              mPage = Page::SETPOINT;  break;
                }
                mPageEdge = true;
                Serial.print(F("[DBG] Carousel -> "));
                Serial.println(static_cast<int>(mPage));
            }
            mDualActive = false;
        }
    }

    /* ===== single-tap handling (debounced) ======================= */
    static uint32_t lastDeb = 0;
    if (mask != mLastMask && now - lastDeb > DEBOUNCE_MS) {

        bool upRel = (mLastMask == 1 && mask == 0);
        bool dnRel = (mLastMask == 2 && mask == 0);

        /* forward ↑ / ↓ to Cal-wizard when we’re on that page ----- */
        if (mPage == Page::CALSCALAR && (upRel || dnRel)) {
            if (UI::Pages::handleCalButton(upRel ? Btn::UP : Btn::DOWN)) {
                lastDeb   = now;
                mLastMask = mask;
                return;                         // wizard consumed
            }
        }

        /* MEASURE page: toggle control mode ----------------------- */
        if (mPage == Page::MEASURE && (upRel || dnRel)) {
            auto cur  = State::read().ctrlMode;
            auto next = (cur == ControlMode::CLOSED) ? ControlMode::OPEN
                                                     : ControlMode::CLOSED;
            State::setCtrlMode(next);
            MainCtrl::onModeChanged(next);
            RGB::flash(LED_BLUE, LED_GREEN);
        }

        /* SETPOINT page edits ------------------------------------- */
        if (mPage == Page::SETPOINT && (upRel || dnRel)) {
            constexpr int32_t STEP_UL_MIN = 25;       // 0.025 mL/min

            if (upRel && mFlowVal <= 64000 - STEP_UL_MIN)
                mFlowVal += STEP_UL_MIN;
            if (dnRel && mFlowVal >= STEP_UL_MIN)
                mFlowVal -= STEP_UL_MIN;

            State::setFlow(static_cast<float>(mFlowVal));

            if (State::read().ctrlMode == ControlMode::CLOSED) {
                announce("Flow", mFlowVal);
            } else {                                  // OPEN loop
                float vpr = State::read().vpr_uL_rev;
                if (vpr < 1.0e-3f) vpr = static_cast<float>(VPR);
                float rpm = static_cast<float>(mFlowVal) / vpr;
                State::setRpm(rpm);
                announce("Flow", mFlowVal);
                announce("RPM",  rpm);
            }
        }

        lastDeb = now;
    }
    mLastMask = mask;

    /* ===== long hold on UP only → system ON/OFF ================== */
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

/* ───────────────── tiny logger ─────────────────────────────*/
void ButtonsTwo::announce(const char* tag, float v) const
{
    Serial.print(F("[BTN] "));
    Serial.print(tag);
    Serial.print(F(": "));
    Serial.println(v, (strcmp(tag, "RPM") == 0 ? 1 : 0));
}

#endif /* ENABLE_BUTTONS_TWO */
