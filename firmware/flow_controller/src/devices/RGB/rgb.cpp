#include "rgb.hpp"
#include <Adafruit_NeoPixel.h>

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
} // namespace

/* ------------------------------------------------------------------ */
void RGB::begin()
{
    /* Power the on-board NeoPixel (FET on GP11) */
    pinMode(PIN_RGB_ENABLE, OUTPUT);
    digitalWrite(PIN_RGB_ENABLE, HIGH);

    strip.begin();
    strip.setBrightness(30);   // adjust 0-255 as needed
    strip.show();              // start OFF
}

/* ------------------------------------------------------------------ */
void RGB::setColour(LEDColour c)
{
    strip.setPixelColor(0, mapColour(c));
    strip.show();
}
