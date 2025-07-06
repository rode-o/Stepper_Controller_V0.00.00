/*
  stepper_mode_test.ino
  • Default: Full-step (M0=M1=M2=Low) @ 200 steps-per-second ≃ 60 RPM
  • Press a key 0-7 in Serial Monitor to select one of the eight
    DRV8825 MODE rows shown in your screenshot.

  XIAO-RP2040 ↔ Pololu DRV8825 wiring (from schematic):
      STEP = D0   DIR  = D1
      EN   = D2   SLP  = D10
      M0   = D3   M1   = D6   M2   = D7
*/

#include <Arduino.h>

/* ── GPIO map ────────────────────────────────────────────────────────── */
constexpr uint8_t PIN_STEP = D0;
constexpr uint8_t PIN_DIR  = D1;
constexpr uint8_t PIN_EN   = D2;
constexpr uint8_t PIN_M0   = D3;
constexpr uint8_t PIN_M1   = D6;
constexpr uint8_t PIN_M2   = D7;
constexpr uint8_t PIN_SLP  = D10;

/* ── Motor constants ────────────────────────────────────────────────── */
constexpr uint16_t FULL_STEPS_PER_REV = 200;   // 1.8° motor
constexpr uint16_t STEP_RATE_SPS      = 200;   // steps per second → 60 RPM
constexpr uint32_t STEP_DELAY_US      = 1'000'000UL / STEP_RATE_SPS / 2;

/* ── MODE table (exactly your screenshot) ───────────────────────────── */
struct ModeRow { bool m0, m1, m2; const char* name; uint8_t div; };

const ModeRow modeTable[8] = {
  /* idx  m0  m1  m2   text           µstep-divisor */
  /* 0 */ {0, 0, 0, "Full step",       1 },
  /* 1 */ {1, 0, 0, "Half step",       2 },
  /* 2 */ {0, 1, 0, "Quarter step",    4 },
  /* 3 */ {1, 1, 0, "Eighth step",     8 },
  /* 4 */ {0, 0, 1, "1/16 step",      16 },
  /* 5 */ {1, 0, 1, "1/32 step (A)",  32 },
  /* 6 */ {0, 1, 1, "1/32 step (B)",  32 },
  /* 7 */ {1, 1, 1, "1/32 step (C)",  32 }
};

uint8_t currentMode = 5;   // start in full-step

/* ── helpers ────────────────────────────────────────────────────────── */
void applyMode(uint8_t idx)
{
  if (idx > 7) return;
  digitalWrite(PIN_M0, modeTable[idx].m0);
  digitalWrite(PIN_M1, modeTable[idx].m1);
  digitalWrite(PIN_M2, modeTable[idx].m2);
  currentMode = idx;
}

void banner()
{
  Serial.print(F("\nMode "));
  Serial.print(currentMode);
  Serial.print(F(": "));
  Serial.println(modeTable[currentMode].name);
}

/* ── SETUP ──────────────────────────────────────────────────────────── */
void setup()
{
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR,  OUTPUT);
  pinMode(PIN_EN,   OUTPUT);
  pinMode(PIN_SLP,  OUTPUT);
  pinMode(PIN_M0,   OUTPUT);
  pinMode(PIN_M1,   OUTPUT);
  pinMode(PIN_M2,   OUTPUT);

  digitalWrite(PIN_DIR, HIGH);    // CW
  digitalWrite(PIN_EN,  LOW);     // driver enabled
  digitalWrite(PIN_SLP, HIGH);    // wake up

  applyMode(currentMode);

  Serial.begin(115200);
  while (!Serial && millis() < 2000) { }
  Serial.println(F("DRV8825 micro-step mode tester"));
  Serial.println(F("Keys 0-7 change mode; default = 0 (Full step)"));
  banner();
}

/* ── LOOP ───────────────────────────────────────────────────────────── */
void loop()
{
  /* ---- Change mode on serial key press ---- */
  if (Serial.available()) {
    char c = Serial.read();
    if (c >= '0' && c <= '7') {
      applyMode(c - '0');
      banner();
      delay(50);   // deburr
    }
  }

  /* ---- Generate one STEP pulse ---- */
  digitalWrite(PIN_STEP, HIGH);
  delayMicroseconds(STEP_DELAY_US);
  digitalWrite(PIN_STEP, LOW);
  delayMicroseconds(STEP_DELAY_US);
}
