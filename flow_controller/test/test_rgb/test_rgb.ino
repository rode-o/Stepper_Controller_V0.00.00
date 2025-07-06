#include <Adafruit_NeoPixel.h>

const uint8_t PIN_RGB_ENABLE = 11;   // GP11 powers the FET
const uint8_t PIN_RGB_DATA   = 12;   // GP12 WS2812 DIN
const uint8_t NUM_PIXELS     = 1;

Adafruit_NeoPixel led(NUM_PIXELS, PIN_RGB_DATA, NEO_GRB + NEO_KHZ800);

/* helper: wheel → 0‒255 maps to RGB */
uint32_t colorWheel(uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85)  return led.Color(255 - pos * 3, 0, pos * 3);          // red→green
  if (pos < 170) { pos -= 85; return led.Color(0, pos * 3, 255 - pos * 3); } // green→blue
  pos -= 170;   return led.Color(pos * 3, 255 - pos * 3, 0);           // blue→red
}

void setup() {
  pinMode(PIN_RGB_ENABLE, OUTPUT);
  digitalWrite(PIN_RGB_ENABLE, HIGH);   // power the pixel

  led.begin();
  led.setBrightness(30);                // scale 0-255
  led.show();
}

void loop() {
  static uint8_t hue = 0;
  led.setPixelColor(0, colorWheel(hue++));
  led.show();
  delay(15);                            // adjust for speed
}
