#pragma once
#include <Adafruit_SH110X.h>

/*  File is three levels below project root:
        ui / pages / page_cal_wizard / page_cal_wizard.hpp
        .. .. ..  ->  ui
*/
#include "../../btn/btn.hpp"               // UI::Btn
#include "../../cal_wizard/cal_wizard.hpp" // CalWizard + gCalWizard
#include "../../../include/_include.hpp"   // SystemState helpers

namespace UI::Pages {

/* draw the current frame of the calibration-wizard page */
void drawCalWizard(Adafruit_SH1107& d,
                   const volatile SystemState& s);

/* periodic tick; call from the main display loop */
void tickCalWizard();

/* pass UP / DOWN / OK coming from ButtonsTwo.
   Returns true if the wizard has consumed the event. */
bool handleCalButton(UI::Btn b);

} // namespace UI::Pages
