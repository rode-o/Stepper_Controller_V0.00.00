/*─────────────────────────────────────────────────────────────*
 *  sh1107.cpp – bootstrap only (no UI logic here)             *
 *─────────────────────────────────────────────────────────────*/
#include "sh1107.hpp"

bool Sh1107Display::begin()
{
    Wire.setClock(400'000);                        // 400 kHz I²C

    if (!mDisp.begin(I2C_ADDR, /*reset=*/true))
        return false;

    /*  rotation 1  → 90 ° clockwise
        After rotating, mDisp.width()==128, height()==64          */
    mDisp.setRotation(1);

    /* sensible defaults */
    mDisp.setTextColor(SH110X_WHITE);
    mDisp.setTextWrap(false);
    mDisp.clearDisplay();
    mDisp.display();
    return true;
}
