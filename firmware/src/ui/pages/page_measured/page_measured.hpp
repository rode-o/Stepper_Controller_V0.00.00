#pragma once
#include <Adafruit_SH110X.h>
#include "../../btn/btn.hpp"
#include "../../../include/_include.hpp"   // SystemState, pins, etc.

namespace UI::Pages
{
    void drawMeasured(Adafruit_SH1107& d,
                      const volatile SystemState& s);
}
