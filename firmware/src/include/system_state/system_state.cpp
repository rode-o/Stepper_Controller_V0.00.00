/*─────────────────────────────────────────────────────────────*/
/*  system_state.cpp – persistence + global state access       */
/*─────────────────────────────────────────────────────────────*/
#include "system_state.hpp"
#include <EEPROM.h>

/* ───── globals ────────────────────────────────────────────── */
volatile SystemState g_state;
bool                 State::g_dirty = false;

/* ───── EEPROM blob layout ─────────────────────────────────── */
static constexpr uint32_t MAGIC   = 0x534D3153;   // "SM1S"
static constexpr uint8_t  VERSION = 2;
static constexpr int      EE_ADDR = 0;

struct PersistBlob {
    uint32_t magic;
    uint8_t  ver;
    float    setFlow_uLmin;
    float    setRpm;
    uint8_t  ctrlMode;
    uint8_t  pumpEnabled;
};

/* ───── read-only accessor ────────────────────────────────── */
const volatile SystemState& State::read() { return g_state; }

/* ───── load persistent settings ──────────────────────────── */
void State::loadPersistent()
{
#if defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
    EEPROM.begin(sizeof(PersistBlob));
#endif
    PersistBlob b{}; EEPROM.get(EE_ADDR, b);

    if (b.magic != MAGIC) return;

    if (b.ver == 1) {                                // migrate old firmware
        g_state.setFlow_uLmin = b.setFlow_uLmin;
        g_state.setRpm        = 30;
        g_state.ctrlMode      = ControlMode::CLOSED;
        g_state.pumpEnabled   = b.pumpEnabled;
        g_dirty = true;                              // write new blob later
    } else if (b.ver == VERSION) {
        g_state.setFlow_uLmin = b.setFlow_uLmin;
        g_state.setRpm        = b.setRpm;
        g_state.ctrlMode      = static_cast<ControlMode>(b.ctrlMode);
        g_state.pumpEnabled   = b.pumpEnabled;
    }
}

/* ───── commit if dirty ────────────────────────────────────── */
void State::commitPersistent()
{
    if (!g_dirty) return;

    PersistBlob b {
        MAGIC, VERSION,
        g_state.setFlow_uLmin,
        g_state.setRpm,
        static_cast<uint8_t>(g_state.ctrlMode),
        static_cast<uint8_t>(g_state.pumpEnabled)
    };
    EEPROM.put(EE_ADDR, b);
#if defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
    EEPROM.commit();
#endif
    g_dirty = false;
}

/* ───── setters that mark EEPROM dirty when needed ─────────── */
void State::setFlow(float v)          { g_state.setFlow_uLmin = v; g_dirty = true; }
void State::setRpm(float v)           { g_state.setRpm        = v; g_dirty = true; }
void State::setCtrlMode(ControlMode m){ g_state.ctrlMode     = m; g_dirty = true; }

/* Reset cumulative totals on rising edge of pump-enable flag */
void State::setPumpEnabled(bool en)
{
    if (en && !g_state.pumpEnabled) {          // OFF → ON transition
        g_state.volume_uL = 0;
        g_state.mass_g    = 0;
    }
    g_state.pumpEnabled = en;
    g_dirty             = true;
}

void State::setSystemOn(bool on)      { g_state.systemOn    = on; }

void State::setRawFlow(float v)       { g_state.r_flow   = v; }
void State::setFiltFlow(float v)      { g_state.f_flow   = v; }
void State::setRPM(float v)           { g_state.rpmCmd   = v; }
void State::setSPS(float v)           { g_state.spsCmd   = v; }
void State::setTop(uint16_t t)        { g_state.topCmd   = t; }
void State::setCalScalar(float p)     { g_state.calScalar= p; }

void State::addVolume(float uL)       { g_state.volume_uL += uL; }
void State::addMass(float g)          { g_state.mass_g   += g; }
void State::setLEDColour(LEDColour c) { g_state.ledColour = c; }

/* ───── getters ────────────────────────────────────────────── */
bool  State::isPumpEnabled() { return g_state.pumpEnabled; }
bool  State::isSystemOn()    { return g_state.systemOn;    }
float State::getCalScalar()  { return g_state.calScalar;   }
