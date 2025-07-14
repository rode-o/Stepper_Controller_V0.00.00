/*─────────────────────────────────────────────────────────────*/
/*  cal_wizard.hpp – UI state-machine for the calibration page */
/*─────────────────────────────────────────────────────────────*/
#pragma once
#include <Adafruit_SH110X.h>
#include <Arduino.h>

#include "../btn/btn.hpp"                 // UI::Btn
#include "../../include/_include.hpp"     // SystemState / State helpers
#include "../../devices/_devices.hpp"     // PumpDrv etc.

namespace UI {

/* ─────────── CalWizard class ───────────────────────────────*/
class CalWizard
{
public:
    /* life-cycle */
    void reset();                 // call once at boot
    void tick();                  // call from loop (≈100 Hz)
    bool handleButton(Btn b);     // true → event consumed

    /* drawing */
    void draw(Adafruit_SH1107& d,
              const volatile SystemState& s) const;

    /* <<<––– **NEW** helper so ButtonsTwo can know when we’re in the
       digit-entry sub-page.  Simply returns (mState == WEIGHT). */
    bool isEditingWeight() const;

private:
    /* ---------- internal state machine ---------- */
    enum class State : uint8_t { INIT, PRIME, READY, RUN, WEIGHT, APPLY };
    void enter(State s);          // state transition helper

    /* state data */
    State     mState     { State::INIT };
    uint32_t  mStageTs   { 0 };
    uint32_t  mRunLeftMs { 0 };
    float     mVolSensor { 0.0f };
    float     mRevCount  { 0.0f };

    /* digit editor */
    uint32_t  mWeightMilli { 0 };   // grams × 1000  (00.000 g ⇒ 0)
    uint8_t   mDigitPos    { 0 };   // 0-4 caret position

    /* constants */
    static constexpr uint16_t PRIME_SECS = 20;
    static constexpr uint16_t CAL_RPM    = 10;
    static constexpr uint32_t RUN_MS     = 600'000;   // 10 min

    /* tiny render helpers */
    void drawInit   (Adafruit_SH1107& d) const;
    void drawPrime  (Adafruit_SH1107& d) const;
    void drawReady  (Adafruit_SH1107& d) const;
    void drawRun    (Adafruit_SH1107& d) const;
    void drawWeight (Adafruit_SH1107& d) const;
    void drawApply  (Adafruit_SH1107& d) const;
};

/* one global instance */
extern CalWizard gCalWizard;

} // namespace UI
