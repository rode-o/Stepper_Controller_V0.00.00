#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "../../../include/_include.hpp"

class Sh1107Display {
public:
    bool  begin();
    void  advancePage();
    void  show(const volatile SystemState& s);

private:
    /* ----- per-page draw helpers ----- */
    void drawSetFlowPage   (const volatile SystemState& s);  // page 0
    void drawMeasuredPage  (const volatile SystemState& s);  // page 1
    void drawCalScalarPage (const volatile SystemState& s);  // page 2  (± Cal %)
    void drawInitCalPage   (const volatile SystemState& s);  // page 3
    void drawCalProgress   ();                               // modal progress bar

    static constexpr uint8_t I2C_ADDR = 0x3C;
    static constexpr uint8_t PAGES    = 4;   // SET | MEAS | CAL% | CAL

    Adafruit_SH1107 mDisp {64, 128, &Wire, -1, 1'000'000};
    uint8_t         mPage = 0;
};
