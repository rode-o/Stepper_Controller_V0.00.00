#pragma once
#include <stdint.h>          // for uint8_t

/* forward-declare to break the include cycle with _ctrl.hpp */
enum class ControlMode : uint8_t;

namespace MainCtrl {
    /* life-cycle */
    void setup();            // call once from Arduino setup()
    void loop100Hz();        // call from Arduino loop()

    /* mode helpers */
    void toggleMode();                       // legacy wrapper
    void onModeChanged(ControlMode newMode); // preferred
}
