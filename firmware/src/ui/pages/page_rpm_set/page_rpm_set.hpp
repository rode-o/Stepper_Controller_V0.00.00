#pragma once
#include <Adafruit_SH110X.h>
#include "../../../include/_include.hpp"
#include "../../btn/btn.hpp"

namespace UI::Pages {
    void drawRpmSet(Adafruit_SH1107& d, const volatile SystemState& s);
    bool handleRpmButton(UI::Btn b);   // returns true if event consumed
    inline void tickRpmSet() {}        // keep for symmetry; no timers yet
}
