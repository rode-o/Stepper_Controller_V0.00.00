#pragma once

#include "../../include/_include.hpp" 
// ^ adjust relative path if your tree differs.

namespace RGB {
    void begin();
    void setColour(LEDColour c);

    /* animation helpers (added earlier) */
    void loopCool();    // CLOSED-mode pump-ON
    void loopWarm();    // OPEN-mode  pump-ON
}
