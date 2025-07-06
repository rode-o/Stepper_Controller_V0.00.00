#pragma once
/* pins.hpp  – central I/O map for the XIAO-RP2040 build
   ───────────────────────────────────────────────────── */

#include <Arduino.h>

/* ── DRV8825 (Pololu) stepper driver ───────────────────────── */
constexpr uint8_t PIN_STEP   = D0;   // pad-1  GP26
constexpr uint8_t PIN_DIR    = D1;   // pad-2  GP27
constexpr uint8_t PIN_EN     = D2;   // pad-3  GP28  (LOW = enable)
constexpr uint8_t PIN_M0     = D3;   // pad-4  GP29
constexpr uint8_t PIN_M1     = D6;   // pad-7  GP0
constexpr uint8_t PIN_M2     = D7;   // pad-8  GP1
constexpr uint8_t PIN_SLEEP  = D10;  // pad-11 GP3  (HIGH = awake)

/* ── Waveshare 1 .3″ OLED (SH1107) ─────────────────────────── */
constexpr uint8_t I2C_SDA_PIN  = D4;     // pad-5  GP6
constexpr uint8_t I2C_SCL_PIN  = D5;     // pad-6  GP7
constexpr uint8_t OLED_I2C_ADDR = 0x3C;  // Waveshare default
constexpr uint8_t OLED_ROTATION = 1;     // 0 = portrait, 1 = landscape-USB-down

/* ── Two user buttons on GP2 / GP4 ─────────────────────────── */
constexpr uint8_t PIN_BTN_UP = D8;   // pad-9  GP2
constexpr uint8_t PIN_BTN_DN = D9;   // pad-10 GP4

/* ── ON-BOARD WS2812 RGB LED ─────────────────────────────── */
constexpr uint8_t PIN_RGB_ENABLE = 11;  // GP11 – powers the NeoPixel FET
constexpr uint8_t PIN_RGB_DATA   = 12;  // GP12 – WS2812 DIN
constexpr uint8_t RGB_NUM_PIXELS = 1;
