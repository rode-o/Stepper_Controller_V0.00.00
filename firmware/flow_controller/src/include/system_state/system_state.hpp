#pragma once
#include <Arduino.h>

/* ─── RGB enum (needed by rgb.hpp) ─── */
enum LEDColour : uint8_t { LED_OFF, LED_RED, LED_GREEN, LED_BLUE, LED_AMBER };

/* ─── Runtime snapshot ───
 * NB: Only setpoint & pumpEnabled persist to EEPROM; all other
 *     members are live-telemetry only.
 */
struct SystemState {
    unsigned long currentTimeMs{0};

    /* user parameters & control */
    float setpoint{0};        // µL / min target
    float calScalar{0};       // user calibration scalar (±%)

    /* flow telemetry */
    float r_flow{0};          // raw   flow  (µL / min)
    float f_flow{0};          // filtered flow

    /* controller commands (live) */
    float rpmCmd{0};
    float spsCmd{0};
    uint16_t topCmd{0};       // ★ NEW: PWM wrap value actually applied

    /* totals */
    float volume_uL{0};
    float mass_g{0};

    /* state flags */
    bool  pumpEnabled{false};
    bool  systemOn{false};
    bool  calibrating{false};

    LEDColour ledColour{LED_OFF};
};

/* global snapshot instance */
extern volatile SystemState g_state;

/* ─── State helpers & persistence ─── */
namespace State {
    extern bool g_dirty;

    /* snapshot read-only accessor */
    const volatile SystemState& read();

    /* EEPROM helpers (store set-point & pump flag only) */
    void loadPersistent();
    void commitPersistent();

    /* setters (some mark EEPROM dirty) */
    void setSetpoint(float v);          // µL/min
    void setPumpEnabled(bool en);
    void setSystemOn(bool on);

    void setRawFlow (float v);
    void setFiltFlow(float v);
    void setRPM(float rpm);
    void setSPS(float sps);
    void setTop(uint16_t top);          // ★ NEW
    void setCalScalar(float p);
    void addVolume(float uL);
    void addMass(float g);
    void setLEDColour(LEDColour c);

    /* getters */
    bool  isPumpEnabled();
    bool  isSystemOn();
    float getCalScalar();
}   // namespace State
