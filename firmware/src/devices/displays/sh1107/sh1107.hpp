/*─────────────────────────────────────────────────────────────*/
/*  sh1107.hpp – 3-page roller UI for 1.3" SH1107 OLED         */
/*─────────────────────────────────────────────────────────────*/
#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "../../../include/_include.hpp"

class Sh1107Display {
public:
    bool  begin();
    void  advancePage();                             // non-animated helper
    void  setPage(uint8_t p) { mPage = p % PAGES; }
    uint8_t currentPage() const { return mPage; }

    void  animatedTo(uint8_t dest,
                     const volatile SystemState& s); // sliding transition
    void  show(const volatile SystemState& s);       // regular redraw

private:
    /* per-page helpers */
    void drawSetPointPage (const volatile SystemState& s);  // index 0
    void drawMeasuredPage (const volatile SystemState& s);  // index 1
    void drawCalScalarPage(const volatile SystemState& s);  // index 2

    /* tiny glyphs */
    static inline void printPlusMinus(Adafruit_SH1107& d)
    { d.print((char)241); }                       // ±
    void drawModeIcon (const volatile SystemState& s);      // top-right
    void drawPumpIcon (const volatile SystemState& s);      // bottom-right

    /* animation helper */
    void renderCurrentPage(const volatile SystemState& s);

    static constexpr uint8_t I2C_ADDR = 0x3C;
    static constexpr uint8_t PAGES    = 3;        // SET | MEAS | CAL

    Adafruit_SH1107 mDisp {64, 128, &Wire, -1, 1'000'000};
    uint8_t         mPage = 0;                    // start on SET page
};
