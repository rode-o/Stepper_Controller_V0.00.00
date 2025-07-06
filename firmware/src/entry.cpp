#include <Arduino.h>
#include "ctrl/main_ctrl/main_ctrl.hpp"

// DO NOT add any extern "…" wrapper here
void setup() {            // matches the core’s `extern "C"` prototype
    MainCtrl::setup();
}

void loop() {             // ditto
    MainCtrl::loop100Hz();
}
