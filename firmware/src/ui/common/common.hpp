#pragma once
#include <Adafruit_SH110X.h>
#include "../../include/_include.hpp"   // 2 × “..” is enough

namespace UI::Pages {

/* shared overlay drawn on *every* page */
void drawCommonOverlay(Adafruit_SH1107& d,
                       const volatile SystemState& s);

} // namespace UI::Pages
