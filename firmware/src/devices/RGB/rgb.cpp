#include "rgb.hpp"
#include "../../include/_include.hpp" 
#include <Adafruit_NeoPixel.h>
#include <math.h>                  // for cosf, fmodf, floorf

namespace {

Adafruit_NeoPixel strip(RGB_NUM_PIXELS, PIN_RGB_DATA,
                        NEO_GRB + NEO_KHZ800);

/* map enum → solid colour */
uint32_t mapColour(LEDColour c)
{
    switch (c) {
        case LED_RED:    return strip.Color(255,   0,   0);
        case LED_GREEN:  return strip.Color(  0, 255,   0);
        case LED_BLUE:   return strip.Color(  0,   0, 255);
        case LED_AMBER:  return strip.Color(255, 100,   0);
        default:         return 0;                       // OFF
    }
}

/* write the same RGB to every pixel */
inline void fillStrip(uint8_t r, uint8_t g, uint8_t b)
{
    for (uint16_t i = 0; i < strip.numPixels(); ++i)
        strip.setPixelColor(i, strip.Color(r, g, b));
    strip.show();
}

} // anonymous namespace

/* ------------------------------------------------------------------ */
/*  PUBLIC API                                                        */
/* ------------------------------------------------------------------ */
void RGB::begin()
{
    /* Power the on-board NeoPixel (FET on GP11) */
    pinMode(PIN_RGB_ENABLE, OUTPUT);
    digitalWrite(PIN_RGB_ENABLE, HIGH);

    strip.begin();
    strip.setBrightness(30);            // adjust 0–255 as needed
    strip.show();                       // start OFF
}

/* set a solid colour immediately */
void RGB::setColour(LEDColour c)
{
    fillStrip((uint8_t)(mapColour(c) >> 16),
              (uint8_t)(mapColour(c) >>  8),
              (uint8_t)(mapColour(c)      ));
}

/* ───── Cool breathing loop (CLOSED mode, pump ON) ───────── */
void RGB::loopCool()
{
    static uint32_t last = 0;
    uint32_t now = millis();
    if (now - last < 20) return;        // 50 Hz guard
    last = now;

    /* cosine breath between aqua ↔ deep blue over 8 s */
    float phase = fmodf(now / 8000.0f, 1.0f);     // 0–1
    float x     = 0.5f - 0.5f * cosf(phase * 2 * M_PI);

    uint8_t r = (uint8_t)(30  * (1.0f - x));      // 30→0→30
    uint8_t g = (uint8_t)(100 + 55  * x);         // 100→155→100
    uint8_t b = (uint8_t)(255 - 155 * x);         // 255→100→255

    fillStrip(r, g, b);
}

/* ───── Warm hue-sweep loop (OPEN mode, pump ON) ─────────── */
void RGB::loopWarm()
{
    static uint32_t last = 0;
    uint32_t now = millis();
    if (now - last < 20) return;        // 50 Hz guard
    last = now;

    /* sweep hue 15°→45° over 7 s, S-V fixed at 1 / 0.8 */
    float phase = fmodf(now / 7000.0f, 1.0f);     // 0–1
    float h = 15.0f + 30.0f * phase;              // degrees
    float s = 1.0f, v = 0.8f;

    /* HSV → RGB */
    int   i = (int)floorf(h / 60.0f);
    float f = h / 60.0f - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));
    float rf, gf, bf;
    switch (i % 6) {
        case 0: rf = v; gf = t; bf = p; break;
        case 1: rf = q; gf = v; bf = p; break;
        case 2: rf = p; gf = v; bf = t; break;
        case 3: rf = p; gf = q; bf = v; break;
        case 4: rf = t; gf = p; bf = v; break;
        default:rf = v; gf = p; bf = q; break;
    }
    fillStrip((uint8_t)(rf * 255), (uint8_t)(gf * 255), (uint8_t)(bf * 255));
}
