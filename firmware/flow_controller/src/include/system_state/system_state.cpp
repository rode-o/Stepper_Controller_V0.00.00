#include "system_state.hpp"
#include <EEPROM.h>                 // AVR / SAMD / Mbed-RP2040 / ESP32

/* ───── single global instance ─────────────────────────── */
volatile SystemState g_state;

/* ───── internal constants ─────────────────────────────── */
static constexpr float     ERR_CLAMP = 50.0f;
static constexpr uint32_t  MAGIC     = 0x53465331;   // "SFS1"
static constexpr uint8_t   VERSION   = 1;
static constexpr int       EE_ADDR   = 0;            // EEPROM offset

/* ───── dirty-flag for deferred writes ─────────────────── */
static bool g_dirty = false;

/* ───── persistent blob layout ─────────────────────────── */
struct PersistBlob {
    uint32_t magic;
    uint8_t  ver;
    float    setpoint_uL;
    float    errorPct;
    uint8_t  controlMode;
    uint8_t  flags;             // bit0 systemOn, bit1 pumpEnabled
};

/* ───── helper: encode flags into one byte ─────────────── */
static uint8_t packFlags()
{
    return static_cast<uint8_t>((g_state.systemOn   ? 0x01 : 0) |
                                (g_state.pumpEnabled? 0x02 : 0));
}

/* ───── public: live snapshot accessor ─────────────────── */
const volatile SystemState& State::read() { return g_state; }

/* ───── public: loadPersistent() ───────────────────────── */
void State::loadPersistent()
{
#if defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
    EEPROM.begin(sizeof(PersistBlob));            // flash-backed cores
#endif
    PersistBlob blob;
    EEPROM.get(EE_ADDR, blob);

    if (blob.magic == MAGIC && blob.ver == VERSION) {
        g_state.setpoint     = blob.setpoint_uL;
        g_state.errorPercent = blob.errorPct;
        g_state.controlMode  = static_cast<ControlMode>(blob.controlMode);
        g_state.systemOn     = blob.flags & 0x01;
        g_state.pumpEnabled  = blob.flags & 0x02;
    }
}

/* ───── public: commitPersistent() ─────────────────────── */
void State::commitPersistent()
{
    if (!g_dirty) return;                       // nothing changed

    PersistBlob blob{
        MAGIC,
        VERSION,
        g_state.setpoint,
        g_state.errorPercent,
        static_cast<uint8_t>(g_state.controlMode),
        packFlags()
    };
    EEPROM.put(EE_ADDR, blob);

#if defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
    EEPROM.commit();                            // flash-backed cores
#endif
    g_dirty = false;
}

/* ───── setters (mark dirty where appropriate) ────────── */
void State::setSetpoint(float uLmin)  { g_state.setpoint  = uLmin; g_dirty = true; }

void State::setErrorPercent(float pct)
{
    if (pct >  ERR_CLAMP) pct =  ERR_CLAMP;
    if (pct < -ERR_CLAMP) pct = -ERR_CLAMP;
    g_state.errorPercent = pct;
    g_dirty = true;
}

void State::setPumpEnabled(bool en) { g_state.pumpEnabled = en; g_dirty = true; }
void State::setSystemOn   (bool on) { g_state.systemOn    = on; g_dirty = true; }
void State::setLEDColour  (LEDColour c) { g_state.ledColour = c; /* not saved */ }
