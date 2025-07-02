/*─────────────────────────────────────────────────────────────*/
/*  sh1107.hpp – 1.3" 128×64 OLED four-page UI                 */
/*─────────────────────────────────────────────────────────────*/

#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "../../../include/_include.hpp"    // LED helpers

class Sh1107Display {
public:
    bool  begin();
    void  advancePage();                 // not used any more but kept
    void  setPage(uint8_t p) { mPage = p % PAGES; }
    uint8_t currentPage() const { return mPage; }

    void  show(const volatile SystemState& s);

private:
    /* per-page helpers */
    void drawModePage     (const volatile SystemState& s);
    void drawSetPointPage (const volatile SystemState& s);
    void drawMeasuredPage (const volatile SystemState& s);
    void drawCalScalarPage(const volatile SystemState& s);

    static inline void printPlusMinus(Adafruit_SH1107& d)
    { d.print((char)241); }              // CP-437 ‘±’

    static constexpr uint8_t I2C_ADDR = 0x3C;
    static constexpr uint8_t PAGES    = 4;      // MODE | SET | MEAS | CAL%

    Adafruit_SH1107 mDisp {64, 128, &Wire, -1, 1'000'000};
    uint8_t         mPage = 0;                  // boot on MODE page
};
