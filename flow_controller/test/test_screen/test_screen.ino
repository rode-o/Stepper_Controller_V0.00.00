/*
  oled_button_modes_landscape_v3.ino
  - no accidental increment/decrement when switching modes -
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

/* ── Display ───────────────────────────────────────────────────── */
Adafruit_SH1107 display(64, 128, &Wire, -1, 1'000'000);

/* ── Button pins ───────────────────────────────────────────────── */
constexpr uint8_t PIN_BTN_UP = D8;   // GP2
constexpr uint8_t PIN_BTN_DN = D9;   // GP4

/* ── Timing constants ─────────────────────────────────────────── */
constexpr uint16_t DEBOUNCE_MS  = 30;
constexpr uint16_t MODE_HOLD_MS = 100;

/* ── App state ────────────────────────────────────────────────── */
enum class Mode : uint8_t { NUMBER, LETTER };
Mode    mode      = Mode::NUMBER;
int32_t numberVal = 0;                    // −9999 … 9999
int8_t  letterIdx = 0;                    // 0 … 25  (A..Z)
constexpr int32_t NUM_MIN = -9999, NUM_MAX = 9999;

/* ── Dual-press FSM ───────────────────────────────────────────── */
enum class LPState : uint8_t { IDLE, COUNTING, ARMED, WAIT_CLEAR };
LPState  lpState   = LPState::IDLE;
uint32_t lpStartMs = 0;

/* ── forward decl. ─────────────────────────────────────────────── */
void redraw();

/* ── SETUP ─────────────────────────────────────────────────────── */
void setup()
{
  Wire.setClock(400'000);
  if (!display.begin(0x3C, true)) while (true) { }
  display.setRotation(1);
  display.setTextColor(SH110X_WHITE);
  display.setTextWrap(false);

  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DN, INPUT_PULLUP);

  redraw();
}

/* ── LOOP ──────────────────────────────────────────────────────── */
void loop()
{
  /* read buttons ------------------------------------------------ */
  bool upLow = digitalRead(PIN_BTN_UP) == LOW;
  bool dnLow = digitalRead(PIN_BTN_DN) == LOW;
  uint8_t mask = (upLow ? 1 : 0) | (dnLow ? 2 : 0);
  static uint8_t lastMask = 0;
  uint32_t now = millis();

  /* dual-press FSM --------------------------------------------- */
  switch (lpState)
  {
    case LPState::IDLE:
      if (mask == 3) { lpState = LPState::COUNTING; lpStartMs = now; }
      break;

    case LPState::COUNTING:
      if (mask != 3)                 lpState = LPState::IDLE;      // gave up
      else if (now - lpStartMs >= MODE_HOLD_MS) lpState = LPState::ARMED;
      break;

    case LPState::ARMED:
      if (mask != 3) {                                          // buttons lifted
        mode = (mode == Mode::NUMBER) ? Mode::LETTER : Mode::NUMBER;
        redraw();
        lpState = LPState::WAIT_CLEAR;                          // stay quiet
      }
      break;

    case LPState::WAIT_CLEAR:
      if (mask == 0) lpState = LPState::IDLE;                   // all released
      break;
  }

  /* single-button actions (only while FSM in IDLE) -------------- */
  if (lpState == LPState::IDLE &&
      mask != lastMask && (now - lpStartMs) > DEBOUNCE_MS)
  {
    /* UP released */
    if (lastMask == 1 && mask == 0) {
      if (mode == Mode::NUMBER && numberVal < NUM_MAX) { ++numberVal; redraw(); }
      if (mode == Mode::LETTER && letterIdx  < 25)     { ++letterIdx;  redraw(); }
    }
    /* DN released */
    if (lastMask == 2 && mask == 0) {
      if (mode == Mode::NUMBER && numberVal > NUM_MIN) { --numberVal; redraw(); }
      if (mode == Mode::LETTER && letterIdx  > 0)      { --letterIdx;  redraw(); }
    }
  }
  lastMask = mask;
}

/* ── redraw UI ─────────────────────────────────────────────────── */
void redraw()
{
  display.clearDisplay();

  display.setFont();
  display.setCursor(0, 0);
  display.print(F("Stepper-Pump UI"));

  display.setCursor(display.width() - 50, 0);
  display.print(mode == Mode::NUMBER ? F("NUM") : F("LET"));

  char buf[12];
  if (mode == Mode::NUMBER) snprintf(buf, sizeof(buf), "%ld", numberVal);
  else                      snprintf(buf, sizeof(buf), "%c", 'A' + letterIdx);

  display.setTextSize(3);
  int16_t bx, by; uint16_t bw, bh;
  display.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
  display.setCursor((display.width() - bw) / 2,
                    (display.height() - bh) / 2);
  display.print(buf);
  display.setTextSize(1);

  display.setCursor(0, display.height() - 8);
  display.print(F("↑/↓ change   hold both = mode"));

  display.display();
}
