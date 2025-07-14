#pragma once
#include <Adafruit_SH110X.h>
#include "../../../include/_include.hpp"
#include "../../btn/btn.hpp"

namespace UI::Pages {

/* render the RPM-SET page */
void drawRpmSet(Adafruit_SH1107& d, const volatile SystemState& s);

/* handle ▲ / ▼ taps – return true if the event was consumed */
bool handleRpmButton(UI::Btn b);

/* no timers yet, but keeps the pattern consistent */
inline void tickRpmSet() {}

} // namespace UI::Pages
