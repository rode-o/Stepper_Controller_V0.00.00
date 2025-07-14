/*─────────────────────────────────────────────────────────────*/
/*  cal_wizard.cpp – calibration wizard with digit editor      */
/*─────────────────────────────────────────────────────────────*/
#include "cal_wizard.hpp"
#include <cstring>              // snprintf
#include <Arduino.h>
#include "../../include/_include.hpp"     // SystemState, State helpers
#include "../../devices/_devices.hpp"     // PumpDrv
#include "../../ui/btn/btn.hpp"           // UI::Btn

namespace UI {

/* ────────── global singleton ───────────────────────────────*/
CalWizard gCalWizard;

/* ────────── local constants ────────────────────────────────*/
static constexpr uint16_t PRIME_RPM = 40;   // hard-coded speed for priming

/* ────────── helpers ────────────────────────────────────────*/
static inline uint32_t incForPos(uint8_t pos)
/* pos 0-4 → 10 000, 1 000, 100, 10, 1 */
{
    static constexpr uint32_t LUT[5] = { 10000, 1000, 100, 10, 1 };
    return LUT[pos % 5];
}

/* ---------------------------------------------------------- */
/*  Internal: enter new sub-state                             */
/* ---------------------------------------------------------- */
void CalWizard::enter(State s)
{
    mState   = s;
    mStageTs = millis();

    /* initialise digit editor when we reach the WEIGHT page */
    if (s == State::WEIGHT) {
        mWeightMilli = 0;        // “00.000 g”
        mDigitPos    = 0;
    }

    /* ----------------------------------------------------------------
       Whenever we drop into PRIME or RUN, hard-set everything:
       - rpm (driver + UI state)
       - clear calibration scalar
       - make sure the pump is enabled
    ------------------------------------------------------------------*/
    switch (s) {
        case State::PRIME:
            ::State::setPumpEnabled(true);
            PumpDrv::setTargetRPM(PRIME_RPM);
            ::State::setRpm(PRIME_RPM);
            ::State::setCalScalar(0.0f);
            break;

        case State::RUN:
            ::State::setPumpEnabled(true);
            PumpDrv::setTargetRPM(CAL_RPM);   // 10 rpm run
            ::State::setRpm(CAL_RPM);
            ::State::setCalScalar(0.0f);
            break;

        default:
            break;
    }
}

/* ---------------------------------------------------------- */
/*  Public helpers                                            */
/* ---------------------------------------------------------- */
void CalWizard::reset() { *this = CalWizard(); }

bool CalWizard::isEditingWeight() const { return mState == State::WEIGHT; }

/* ---------------------------------------------------------- */
/*  Main tick                                                 */
/* ---------------------------------------------------------- */
void CalWizard::tick()
{
    uint32_t now = millis();

    switch (mState)
    {
        /* 20 s prime → READY ---------------------------------------*/
        case State::PRIME:
            if (now - mStageTs >= PRIME_SECS * 1000UL) {
                ::State::setPumpEnabled(false);
                PumpDrv::stop();
                enter(State::READY);
            }
            break;

        /* 10-min run → WEIGHT --------------------------------------*/
        case State::RUN:
            if (mRunLeftMs > 0) {
                uint32_t dt = now - mStageTs;
                mStageTs    = now;
                mRunLeftMs  = (dt >= mRunLeftMs) ? 0 : mRunLeftMs - dt;

                mVolSensor += ::State::read().f_flow * (dt / 60000.0f); // µL
                mRevCount  += (CAL_RPM * dt) / 60000.0f;             // rev
            }
            if (mRunLeftMs == 0) {
                ::State::setPumpEnabled(false);
                PumpDrv::stop();
                enter(State::WEIGHT);
            }
            break;

        /* 2-s “Cal OK” toast --------------------------------------*/
        case State::APPLY:
            if (now - mStageTs >= 2000) enter(State::INIT);
            break;

        default: break;
    }
}

/* ---------------------------------------------------------- */
/*  Button handler – unchanged apart from rpm constants        */
/* ---------------------------------------------------------- */
bool CalWizard::handleButton(Btn b)
{
    switch (mState)
    {
        /* ── 1. INIT ─────────────────────────────────────────────*/
        case State::INIT:
            if (b == Btn::OK) {
                enter(State::PRIME);               // enter() sets 40 rpm
                return true;
            }
            return false;

        /* ── 2. READY ───────────────────────────────────────────*/
        case State::READY:
            if (b == Btn::UP) {                    // start run
                mVolSensor = mRevCount = 0.0f;
                mRunLeftMs = RUN_MS;
                enter(State::RUN);                 // enter() sets 10 rpm
            }
            else if (b == Btn::DOWN) {             // re-prime
                enter(State::PRIME);               // back to 40 rpm
            }
            return true;

        /* ── 3. WEIGHT editor – identical to before ─────────────*/
        case State::WEIGHT: {
            uint32_t inc = incForPos(mDigitPos);

            switch (b)
            {
                case Btn::UP:
                    if (mWeightMilli + inc <= 99999) mWeightMilli += inc;
                    return true;

                case Btn::DOWN:
                    if (mWeightMilli >= inc) mWeightMilli -= inc;
                    return true;

                case Btn::NEXT_DIGIT:
                    mDigitPos = (mDigitPos + 1) % 5;
                    return true;

                case Btn::OK: {
                    /* 1. operator’s measured mass → true delivered volume -------- */
                    float grams = mWeightMilli / 1000.0f;          // XX.XXX g
                    constexpr float RHO = 0.997f;                 // g mL⁻¹  (water @25 °C)
                    float V_true_uL = grams / RHO * 1000.0f;      // µL

                    /* 2. sensor volume & pump revs collected during RUN ---------- */
                    float k_gain   = (mVolSensor > 1.0f)          // absolute gain (≈1.05 if 5 % low)
                                ? V_true_uL / mVolSensor
                                : 1.0f;

                    /* 3. convert gain → ± percent error that readFlow() wants ---- */
                    float cal_pct  = (k_gain - 1.0f) * 100.0f;    // +5.26 % in the same example

                    /* 4. store results ------------------------------------------- */
                    ::State::setCalScalar(cal_pct);               // <-- now a percent, not a gain
                    ::g_state.vpr_uL_rev = (mRevCount > 0.1f)     // µL per pump rev (for UI / logs)
                                        ? V_true_uL / mRevCount
                                        : 0.0f;
                    ::State::g_dirty = true;                      // mark EEPROM save

                    enter(State::APPLY);                          // show “Cal OK” toast
                    return true;
                }
                default: return false;
            }
        }

        default: return false;
    }
}

/* ---------------------------------------------------------- */
/*  Draw dispatcher & tiny renders (unchanged)                */
/* ---------------------------------------------------------- */
void CalWizard::draw(Adafruit_SH1107& d,
                     const volatile SystemState&) const
{
    switch (mState) {
        case State::INIT:   drawInit   (d); break;
        case State::PRIME:  drawPrime  (d); break;
        case State::READY:  drawReady  (d); break;
        case State::RUN:    drawRun    (d); break;
        case State::WEIGHT: drawWeight (d); break;
        case State::APPLY:  drawApply  (d); break;
    }
}

/* ---------------------------------------------------------- */
/*  Tiny renders (unchanged ones folded for brevity)          */
/* ---------------------------------------------------------- */
void CalWizard::drawInit(Adafruit_SH1107& d) const
{
    d.clearDisplay(); d.setFont();
    d.setCursor(16, 18);
    d.setTextSize(2);
    d.print(F("Init cal?"));
    d.setTextSize(1);
    d.setCursor(18, 46);
    d.print(F("Hold both 5s"));
}

void CalWizard::drawPrime(Adafruit_SH1107& d) const
{
    uint32_t elapsed = (millis() - mStageTs) / 1000;
    uint32_t left    = PRIME_SECS > elapsed ? PRIME_SECS - elapsed : 0;

    d.clearDisplay(); d.setFont();
    d.setCursor(20, 24);
    d.setTextSize(2);
    d.print(F("Prime"));
    d.setTextSize(1);
    d.setCursor(52, 46);
    d.print(left); d.print('s');
}

void CalWizard::drawReady(Adafruit_SH1107& d) const
{
    d.clearDisplay(); d.setFont();
    d.setCursor(12, 18);
    d.setTextSize(2);
    d.print(F("Ready?"));
    d.setTextSize(1);
    d.setCursor(4, 46);
    d.print(F("▲ Cal   ▼ Prime"));
}

void CalWizard::drawRun(Adafruit_SH1107& d) const
{
    uint32_t min = mRunLeftMs / 60000;
    uint32_t sec = (mRunLeftMs % 60000) / 1000;

    char buf[8];
    snprintf(buf, sizeof buf, "%02lu:%02lu",
             static_cast<unsigned long>(min),
             static_cast<unsigned long>(sec));

    d.clearDisplay(); d.setFont();
    d.setTextSize(2);
    int16_t bx, by; uint16_t bw, bh;
    d.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    d.setCursor((d.width() - bw) / 2, 20);
    d.print(buf);
    d.setTextSize(1);
    d.setCursor(36, 46);
    d.print(F("CAL"));
}

void CalWizard::drawWeight(Adafruit_SH1107& d) const
{
    d.clearDisplay(); d.setFont();
    d.setCursor(2, 0); d.print(F("Enter g:"));

    /* format “XX.XXX” */
    char buf[8];
    snprintf(buf, sizeof buf, "%02lu.%03lu",
             static_cast<unsigned long>(mWeightMilli / 1000),
             static_cast<unsigned long>(mWeightMilli % 1000));

    d.setTextSize(2);
    d.setCursor(12, 24);
    d.print(buf);
    d.setTextSize(1);

    /* caret underline under current digit */
    constexpr int8_t caretX[5] = { 12, 28, 44, 60, 68 };
    constexpr int8_t caretY    = 44;
    d.drawFastHLine(caretX[mDigitPos], caretY, 12, SH110X_WHITE);
}

void CalWizard::drawApply (Adafruit_SH1107& d) const {/* … */}

} // namespace UI
