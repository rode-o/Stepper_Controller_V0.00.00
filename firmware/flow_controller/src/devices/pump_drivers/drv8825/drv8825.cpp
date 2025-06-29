#include "drv8825.hpp"

#ifdef ENABLE_DRV8825
using namespace PumpDrv;

/* 1/32 µ-step pattern (M0 tied LOW on carrier) */
struct Mode { bool m0, m1, m2; };
static constexpr Mode MODE_1_32{ false, true, true };

/* ───── internal state ───── */
namespace {
    float  tgtSps = 0.0f;
    float  curSps = 0.0f;
    constexpr float PULSES_PER_REV = 200.0f * MICROSTEP_DIV;

#if !(defined(ARDUINO_ARCH_RP2040) || defined(__AVR__))
    /* bit-bang backend variables */
    volatile uint32_t halfPeriodUs = 0;
    volatile bool     stepLevel    = false;
    uint32_t          lastToggleUs = 0;
#endif
}

/* ───── helpers ───── */
static void setMicrostepPins(const Mode& m)
{
    digitalWrite(PIN_M1, m.m1);
    digitalWrite(PIN_M2, m.m2);
}
static void driverEnable(bool en)
{
    digitalWrite(PIN_EN   , en ? LOW  : HIGH);
    digitalWrite(PIN_SLEEP, en ? HIGH : LOW );
}

/* ─────────────────────────── RP2040 PWM slice backend ─────────────────────────── */
#if defined(ARDUINO_ARCH_RP2040)
#include "hardware/pwm.h"
static uint slice;

static inline void hwSetTop(uint16_t top)
{
    if (top == 0) {                      // stop pulses, tri-state STEP
        pwm_set_enabled(slice, false);
        return;
    }
    pwm_set_wrap(slice, top);
    pwm_set_chan_level(slice, PWM_CHAN_A, top / 2);   // 50 % duty
    pwm_set_enabled(slice, true);                      // always ensure enabled
}

static inline void hwSetFreq(uint32_t sps)             /* legacy helper */
{
    if (!sps) { hwSetTop(0); return; }
    uint32_t clk = clock_get_hz(clk_sys);
    uint32_t top = clk / sps;
    if (top) top -= 1; else top = 1;
    hwSetTop(static_cast<uint16_t>(top));
}

/* ─────────────────────────── AVR 16-bit Timer1 backend ─────────────────────────── */
#elif defined(__AVR__)
static inline void hwSetTop(uint16_t top)
{
    if (top == 0) { TCCR1B = 0; return; }
    pinMode(PIN_STEP, OUTPUT);
    TCCR1A = _BV(COM1A0);                       // toggle OC1A on compare
    TCCR1B = _BV(WGM12) | _BV(CS10);            // CTC, presc=1
    OCR1A  = top;
}
static inline void hwSetFreq(uint32_t sps)
{
    if (!sps) { hwSetTop(0); return; }
    uint32_t top = (F_CPU / (2UL * sps)) - 1;
    if (top > 0xFFFF) top = 0xFFFF;
    hwSetTop(static_cast<uint16_t>(top));
}

/* ─────────────────────────── bit-bang fallback backend ─────────────────────────── */
#else
static inline void hwSetTop(uint16_t top)
{
    halfPeriodUs = top ? (top + 1) / 2 : 0;
}
static inline void hwSetFreq(uint32_t sps)
{
    halfPeriodUs = sps ? 500'000UL / sps : 0;   // µs half-period
}
static inline void hwService()
{
    if (!halfPeriodUs) return;
    uint32_t now = micros();
    if (now - lastToggleUs >= halfPeriodUs) {
        lastToggleUs = now;
        stepLevel = !stepLevel;
        digitalWrite(PIN_STEP, stepLevel);
    }
}
#endif  /* backend selection */

/* ───── API implementation ───── */
void PumpDrv::initPump()
{
    pinMode(PIN_STEP , OUTPUT);
    pinMode(PIN_DIR  , OUTPUT);
    pinMode(PIN_EN   , OUTPUT);
    pinMode(PIN_SLEEP, OUTPUT);

#if defined(ARDUINO_ARCH_RP2040)
    pinMode(PIN_M0, INPUT_PULLDOWN);
#else
    pinMode(PIN_M0, INPUT); digitalWrite(PIN_M0, LOW);
#endif
    pinMode(PIN_M1, OUTPUT); pinMode(PIN_M2, OUTPUT);

    setMicrostepPins(MODE_1_32);
    digitalWrite(PIN_DIR, HIGH);
    driverEnable(false);

#if defined(ARDUINO_ARCH_RP2040)
    slice = pwm_gpio_to_slice_num(PIN_STEP);
    pwm_config cfg = pwm_get_default_config();
    pwm_init(slice, &cfg, false);
    gpio_set_function(PIN_STEP, GPIO_FUNC_PWM);
#endif
}

/* ---- legacy speed-ramp interface (kept for bit-bang & AVR modes) ---- */
void PumpDrv::setTargetSPS(float sps)
{
    if (sps < 0) sps = 0;
    if (sps > MAX_SPS) sps = MAX_SPS;
    tgtSps = sps;
}
void PumpDrv::setTargetRPM(float rpm)
{ setTargetSPS((rpm / 60.0f) * PULSES_PER_REV); }

float PumpDrv::currentSPS() { return curSps; }

void PumpDrv::pumpService()
{
#if !(defined(ARDUINO_ARCH_RP2040) || defined(__AVR__))
    hwService();                                  // tick bit-bang
#endif
    if constexpr (ACCEL_SPS_PER_CYCLE) {
        if (curSps < tgtSps)      curSps = min(curSps + ACCEL_SPS_PER_CYCLE, tgtSps);
        else if (curSps > tgtSps) curSps = max(curSps - ACCEL_SPS_PER_CYCLE, tgtSps);
    } else curSps = tgtSps;

    if (curSps < MIN_SPS) { driverEnable(false); hwSetFreq(0); return; }
    driverEnable(true);  hwSetFreq(static_cast<uint32_t>(curSps));
}

/* ---- period-driven API (preferred on RP2040) ---- */
void PumpDrv::setTop(uint16_t top)
{
    if (top == 0) driverEnable(false);
    else          driverEnable(true);
    hwSetTop(top);
}
#endif  /* ENABLE_DRV8825 */
