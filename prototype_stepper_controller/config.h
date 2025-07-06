
#pragma once
#include <Arduino.h>

constexpr uint8_t STEP_PIN  = D0;   // GP2
constexpr uint8_t DIR_PIN   = D1;   // GP3
constexpr uint8_t EN_PIN    = D2;   // GP4 (active LOW)

constexpr uint16_t STEPS_PER_REV = 200;  // motor spec
constexpr uint8_t MICROSTEP = 32;        // jumper setting (1/32)
