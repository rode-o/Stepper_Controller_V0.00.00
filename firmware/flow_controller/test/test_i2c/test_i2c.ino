/*
  i2c_bus_scan.ino
  -----------------
  • Scans addresses 0x08-0x77 once per second.
  • Prints a table to Serial.
  • Shows “I2C Scan” and the list on the 128×64 SH1107 OLED.

  XIAO RP2040 pinout (your schematic):
      SDA  = D4  (GP6)
      SCL  = D5  (GP7)
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

/* ── OLED: 128×64, I²C addr 0x3C ─────────────────────────────────── */
Adafruit_SH1107 oled(64, 128, &Wire, -1, 1000000);   // 1 MHz I²C

/* ── Forward decl. ───────────────────────────────────────────────── */
void scanAndPrint();
void drawToOLED(uint8_t found[], uint8_t count);

/* ── SETUP ───────────────────────────────────────────────────────── */
void setup()
{
  Serial.begin(115200);
  while (!Serial && millis() < 2000) { }

  /* I²C @ 400 kHz for faster scan */
  Wire.setClock(400000);
  Wire.begin();

  if (!oled.begin(0x3C, true)) {            // addr, reset_pin = -1
    Serial.println("OLED not found (continuing headless)");
  } else {
    oled.setTextColor(SH110X_WHITE);
    oled.setTextWrap(false);
  }

  Serial.println("\n=== I2C BUS SCANNER (XIAO RP2040) ===");
}

/* ── LOOP: repeat once per second ───────────────────────────────── */
void loop()
{
  scanAndPrint();
  delay(1000);
}

/* ── Scan helper ─────────────────────────────────────────────────── */
void scanAndPrint()
{
  uint8_t found[16];         // up to 16 devices
  uint8_t cnt = 0;

  Serial.print("\nScan @ ");
  Serial.print(millis() / 1000);
  Serial.println(" s");
  Serial.println("+----+");

  for (uint8_t addr = 0x08; addr <= 0x77; addr++)
  {
    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission();

    if (err == 0) {                            // device ACKed
      if (cnt < sizeof(found)) found[cnt++] = addr;
      Serial.print("| 0x");
      if (addr < 0x10) Serial.print('0');
      Serial.print(addr, HEX);
      Serial.println(" |");
    }
  }
  if (cnt == 0) Serial.println("| none |");
  Serial.println("+----+");

  if (oled.width()) drawToOLED(found, cnt);    // only if OLED present
}

/* ── OLED render ─────────────────────────────────────────────────── */
void drawToOLED(uint8_t found[], uint8_t count)
{
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.setTextSize(1);
  oled.println("I2C Scan");

  oled.setCursor(0, 12);
  if (count == 0) {
    oled.println("No devices");
  } else {
    for (uint8_t i = 0; i < count; ++i) {
      oled.printf("0x%02X  ", found[i]);
      if ((i & 3) == 3) oled.println();        // 4 per line
    }
  }
  oled.display();
}
