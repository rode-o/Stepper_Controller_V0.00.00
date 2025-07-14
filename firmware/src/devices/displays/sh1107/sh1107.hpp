#pragma once
#include <Adafruit_SH110X.h>
#include <Wire.h>
#include <Arduino.h>

/*─────────────────────────────────────────────────────────────*
 *  Sh1107Display – minimal wrapper.                           *
 *  Keeps UI code independent of the Adafruit class details.   *
 *  Panel: Waveshare Pico-OLED-1.3 (SH1107G, 128×64 glass)     *
 *─────────────────────────────────────────────────────────────*/
class Sh1107Display {
public:
    /* Call once from setup(); returns false if the panel is missing */
    bool begin();

    /* optional direct access for advanced drawing */
    Adafruit_SH1107& raw()                    { return mDisp; }

    /* convenience – most UI code only needs these */
    void     clear()                { mDisp.clearDisplay(); }
    void     display()              { mDisp.display();      }
    uint16_t width()   const        { return mDisp.width(); }
    uint16_t height()  const        { return mDisp.height(); }

private:
    /* Waveshare boards are wired for 0x3C; change to 0x3D if required */
    static constexpr uint8_t I2C_ADDR = 0x3C;

    /* 64×128 buffer + rotation-1 → logical 128×64 landscape */
    Adafruit_SH1107 mDisp{64, 128, &Wire};
};
