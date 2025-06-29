/*  system_state.cpp – runtime snapshot & persistence
 *  --------------------------------------------------
 *  EEPROM blob stores ONLY:
 *    • set-point (float, µL/min)
 *    • pump flag (uint8_t, 0|1)
 *  Live-only telemetry carries sensor & controller data.
 */

#include "system_state.hpp"
#include <EEPROM.h>

/* ───────── global snapshot & dirty flag ───────── */
volatile SystemState g_state;
bool                 State::g_dirty = false;

/* ───────── EEPROM layout ───────── */
static constexpr uint32_t MAGIC   = 0x534D3153;   // "SM1S"
static constexpr uint8_t  VERSION = 1;
static constexpr int      EE_ADDR = 0;

struct PersistBlob {
    uint32_t magic;
    uint8_t  ver;
    float    setpoint_uL;
    uint8_t  pumpEnabled;
};

/* ───────── public helpers ───────── */
const volatile SystemState& State::read() { return g_state; }

/* Load set-point & pump flag from flash-backed EEPROM */
void State::loadPersistent()
{
#if defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
    EEPROM.begin(sizeof(PersistBlob));
#endif
    PersistBlob blob{}; EEPROM.get(EE_ADDR, blob);

    if (blob.magic == MAGIC && blob.ver == VERSION) {
        g_state.setpoint    = blob.setpoint_uL;
        g_state.pumpEnabled = blob.pumpEnabled;
    }
}

/* Flush when dirty (only set-point & pump flag) */
void State::commitPersistent()
{
    if (!g_dirty) return;

    PersistBlob blob{
        MAGIC, VERSION,
        g_state.setpoint,
        static_cast<uint8_t>(g_state.pumpEnabled)
    };
    EEPROM.put(EE_ADDR, blob);

#if defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
    EEPROM.commit();
#endif
    g_dirty = false;
}

/* ───────── setters ───────── */
void State::setSetpoint   (float v)   { g_state.setpoint    = v;   g_dirty = true; }
void State::setPumpEnabled(bool e)    { g_state.pumpEnabled = e;   g_dirty = true; }

void State::setSystemOn(bool on)      { g_state.systemOn   = on; }

void State::setRawFlow (float v)      { g_state.r_flow     = v; }
void State::setFiltFlow(float v)      { g_state.f_flow     = v; }
void State::setRPM(float rpm)         { g_state.rpmCmd     = rpm; }
void State::setSPS(float sps)         { g_state.spsCmd     = sps; }
void State::setTop(uint16_t top)      { g_state.topCmd     = top; }   // ★ NEW
void State::setCalScalar(float p)     { g_state.calScalar  = p; }

void State::addVolume(float uL)       { g_state.volume_uL += uL; }
void State::addMass(float g)          { g_state.mass_g    += g; }
void State::setLEDColour(LEDColour c) { g_state.ledColour = c; }

/* ───────── getters ───────── */
bool  State::isPumpEnabled() { return g_state.pumpEnabled; }
bool  State::isSystemOn()    { return g_state.systemOn;    }
float State::getCalScalar()  { return g_state.calScalar;   }
