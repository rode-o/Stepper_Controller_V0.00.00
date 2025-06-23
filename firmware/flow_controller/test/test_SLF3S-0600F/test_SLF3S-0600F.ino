/*
   SLF3S-0600F Quick-Test Sketch
   ─────────────────────────────
   • Initialises the Sensirion SLF3S-0600F in H₂O continuous mode
   • Prints:  raw µL/min, compensated mL/min, temperature °C, status flags
   • Serial command:  e=<value>   (e.g.  e=5  → +5 % calibration)
   • On sensor I²C failure the built-in LED blinks rapidly
*/

#include <Wire.h>
#include <SensirionI2cSf06Lf.h>

/* ───── compile-time constants ───────────────────────────────────────────*/
constexpr uint8_t  FLOW_ADDR  = SLF3S_0600F_I2C_ADDR_08;       // 0x08
constexpr uint8_t  INV_SCALE  = INV_FLOW_SCALE_FACTORS_SLF3S_0600F; // 10
constexpr uint16_t WARMUP_FRAMES = 3;                          // skip n samples
constexpr uint32_t PRINT_INTERVAL_MS = 250;

/* ───── globals ──────────────────────────────────────────────────────────*/
SensirionI2cSf06Lf flow;

float     raw_uLmin   = 0.0f;
float     raw_tempC   = 0.0f;
uint16_t  flags       = 0;
uint16_t  readCount   = 0;

float     errPercent  = 0.0f;     // editable from serial (-50 … +50)

/* ───── helper ───────────────────────────────────────────────────────────*/
float getCompensated_mLmin(float uLmin)
{
    float factor = 1.0f / (1.0f - errPercent / 100.0f);
    return (uLmin * factor) / 1000.0f;   // → mL/min
}

/* ───── setup ────────────────────────────────────────────────────────────*/
void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    Wire.begin();

    flow.begin(Wire, FLOW_ADDR);
    flow.stopContinuousMeasurement();
    delay(5);

    int16_t err = flow.startH2oContinuousMeasurement();
    if (err) {
        Serial.print(F("Sensor start error: "));
        Serial.println(err);
        /* Blink LED fast to indicate failure */
        while (true) {
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            delay(100);
        }
    }
    Serial.println(F("SLF3S-0600F init OK"));
}

/* ───── loop ─────────────────────────────────────────────────────────────*/
void loop()
{
    /* ---- 1. optional serial command: e=<val> -------------------------- */
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd.startsWith("e=")) {
            errPercent = cmd.substring(2).toFloat();
            Serial.print(F("calibration set to "));
            Serial.print(errPercent, 1);
            Serial.println(F(" %"));
        }
    }

    /* ---- 2. sensor read ---------------------------------------------- */
    static uint32_t tLastPrint = 0;
    if (millis() - tLastPrint >= PRINT_INTERVAL_MS) {
        tLastPrint = millis();

        int16_t err = flow.readMeasurementData(
                          INV_FLOW_SCALE_FACTORS_SLF3S_0600F,
                          raw_uLmin,
                          raw_tempC,
                          flags);
        if (err == 0 && ++readCount > WARMUP_FRAMES) {
            float comp_mLmin = getCompensated_mLmin(raw_uLmin);

            Serial.print(F("raw="));
            Serial.print(raw_uLmin, 0);   Serial.print(F(" µL/min,  "));
            Serial.print(F("comp="));
            Serial.print(comp_mLmin, 3);  Serial.print(F(" mL/min,  "));
            Serial.print(F("T="));
            Serial.print(raw_tempC, 1);   Serial.print(F(" °C,  "));
            Serial.print(F("flags=0x"));
            Serial.println(flags, HEX);
        } else if (err) {
            Serial.print(F("read error: "));
            Serial.println(err);
        }
    }
}
