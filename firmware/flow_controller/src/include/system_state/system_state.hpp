#pragma once
#include <Arduino.h>

/* ───── RGB status ─────────────────────────────────────────── */
enum LEDColour : uint8_t { LED_OFF, LED_RED, LED_GREEN, LED_BLUE, LED_AMBER };

/* ───── Control-mode enum ─────────────────────────────────── */
enum ControlMode : uint8_t { CONTROL_MODE_EXP = 0,
                             CONTROL_MODE_CONST_VOLTAGE };

/* ───── Global runtime snapshot ───────────────────────────── */
struct SystemState {
    /* timing ------------------------------------------------ */
    unsigned long currentTimeMs{0};

    /* sensors ----------------------------------------------- */
    float flow{0};
    float setpoint{0};
    float errorPercent{0};
    float temperature{0};
    bool  bubbleDetected{false};

    /* flags ------------------------------------------------- */
    bool systemOn{false};
    ControlMode controlMode{CONTROL_MODE_EXP};

    /* controller diagnostics -------------------------------- */
    float pidOutput{0}, desiredVoltage{0};
    float pTerm{0}, iTerm{0}, dTerm{0};
    float pGain{0}, iGain{0}, dGain{0};
    float filteredError{0}, currentAlpha{0};

    /* pump & valve ------------------------------------------ */
    bool pumpEnabled{false};
    bool pumpReverse{false};

    /* driver commands --------------------------------------- */
    float rpmCmd{0};
    float spsCmd{0};

    /* shared LED colour ------------------------------------- */
    LEDColour ledColour{LED_OFF};
    
    /* cumulative totals ------------------------------------ */
    float volume_uL{0};           // running total in µL
    float mass_g{0};              // running total in g (uses density)
};

/* ───── namespace State — zero-copy access helpers ───────── */
namespace State {
    const volatile SystemState& read();          // implemented in state.cpp

    /* setters */
    void  setErrorPercent(float pct);            // clamps to ±50
    void  setPumpEnabled(bool en);
    void  setSystemOn(bool on);
    void  setLEDColour(LEDColour c);

    /* inline trivial getters (no function-call overhead) */
    inline float     getErrorPercent() { return read().errorPercent; }
    inline bool      isPumpEnabled()  { return read().pumpEnabled;  }
    inline bool      isSystemOn()     { return read().systemOn;     }
    inline LEDColour getLEDColour()   { return read().ledColour;    }
}
