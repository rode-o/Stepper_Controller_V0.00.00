#pragma once
#include <Arduino.h>

enum LEDColour : uint8_t { LED_OFF, LED_RED, LED_GREEN, LED_BLUE, LED_AMBER };
enum class ControlMode : uint8_t { CLOSED = 0, OPEN = 1 };

struct SystemState {
    unsigned long currentTimeMs{0};

    /* user parameters */
    float setFlow_uLmin{500};          // CLOSED mode target
    float setRpm       {30};           // OPEN mode target
    float calScalar    {0};

    ControlMode ctrlMode{ControlMode::CLOSED};

    /* flow telemetry */
    float r_flow{0}, f_flow{0};

    /* driver commands */
    float rpmCmd{0}, spsCmd{0};
    uint16_t topCmd{0};

    /* totals */
    float volume_uL{0}, mass_g{0};

    /* flags */
    bool pumpEnabled{false};
    bool systemOn   {false};

    LEDColour ledColour{LED_OFF};
};

extern volatile SystemState g_state;

namespace State {
    extern bool g_dirty;
    const volatile SystemState& read();

    void loadPersistent();
    void commitPersistent();

    void setFlow(float v);
    void setRpm (float v);
    void setCtrlMode(ControlMode m);

    void setPumpEnabled(bool en);
    void setSystemOn(bool on);

    void setRawFlow(float v);
    void setFiltFlow(float v);
    void setRPM(float rpm);
    void setSPS(float sps);
    void setTop(uint16_t top);
    void setCalScalar(float p);
    void addVolume(float uL);
    void addMass(float g);
    void setLEDColour(LEDColour c);

    bool  isPumpEnabled();
    bool  isSystemOn();
    float getCalScalar();
}
