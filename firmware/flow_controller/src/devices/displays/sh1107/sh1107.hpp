#pragma once
/*  SH1107 OLED view layer – 3-page flow UI
    ─────────────────────────────────────── */
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "../../../include/_include.hpp"

class Sh1107Display {
public:
    bool  begin();                                   // call once in initDisplay()
    void  advancePage();                             // rising edge from buttons
    void  show(const volatile SystemState& s);       // call every main loop()

private:
    /* page-specific render helpers (all accept volatile) */
    void drawSetFlowPage  (const volatile SystemState& s);  // page 1 – set-point
    void drawMeasuredPage (const volatile SystemState& s);  // page 2 – measured flow
    void drawErrorCalPage (const volatile SystemState& s);  // page 3 – error %

    static constexpr uint8_t I2C_ADDR = 0x3C;
    static constexpr uint8_t PAGES    = 3;

    Adafruit_SH1107 mDisp {64, 128, &Wire, -1, 1'000'000};
    uint8_t         mPage = 0;
};
