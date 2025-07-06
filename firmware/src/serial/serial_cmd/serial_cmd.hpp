#pragma once
#include <Arduino.h>
#include "../../include/system_state/system_state.hpp"

namespace SerialCmd {
    void begin(uint32_t baud = 115200);   // call once in setup()
    void poll();                          // call from fast loop
}  // namespace SerialCmd
