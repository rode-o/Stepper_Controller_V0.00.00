#include "system_state.hpp"

/* ───── single global instance ───────────────────────────── */
/* NOTE: external linkage so other translation units can see it */
volatile SystemState g_state;

static constexpr float ERR_CLAMP = 50.0f;

/* ───── accessors ────────────────────────────────────────── */
const volatile SystemState& State::read() { return g_state; }

/* ---------- setters ----------- */
void State::setErrorPercent(float pct)
{
    if (pct >  ERR_CLAMP) pct =  ERR_CLAMP;
    if (pct < -ERR_CLAMP) pct = -ERR_CLAMP;
    g_state.errorPercent = pct;
}

void State::setPumpEnabled(bool en)   { g_state.pumpEnabled = en; }
void State::setSystemOn(bool on)      { g_state.systemOn    = on; }
void State::setLEDColour(LEDColour c) { g_state.ledColour   = c; }
